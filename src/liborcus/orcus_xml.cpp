/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_xml.hpp>
#include <orcus/sax_ns_parser.hpp>
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/export_interface.hpp>
#include <orcus/stream.hpp>
#include <orcus/string_pool.hpp>

#include "orcus_xml_impl.hpp"

#define ORCUS_DEBUG_XML 0

#if ORCUS_DEBUG_XML
#include <iostream>
#endif

#include <vector>
#include <fstream>

namespace orcus {

namespace {

class xml_data_sax_handler
{
    struct scope
    {
        xml_name_t name;
        std::ptrdiff_t element_open_begin;
        std::ptrdiff_t element_open_end;

        xml_map_tree::element_type type;

        scope(xmlns_id_t _ns, std::string_view _name) :
            name(_ns, _name),
            element_open_begin(0),
            element_open_end(0),
            type(xml_map_tree::element_type::unknown) {}
    };

    std::vector<sax_ns_parser_attribute> m_attrs;
    std::vector<scope> m_scopes;

    string_pool m_pool;
    spreadsheet::iface::import_factory& m_factory;
    xml_map_tree::const_element_list_type& m_link_positions;
    const xml_map_tree& m_map_tree;
    xml_map_tree::walker m_map_tree_walker;

    xml_map_tree::element* mp_current_elem;
    std::string_view m_current_chars;
    bool m_in_range_ref;
    xml_map_tree::range_reference* mp_increment_row;

private:

    const sax_ns_parser_attribute* find_attr_by_name(const xml_name_t& name)
    {
        for (const sax_ns_parser_attribute& attr : m_attrs)
        {
            if (attr.ns == name.ns && attr.name == name.name)
                return &attr;
        }
        return nullptr;
    }

    void set_single_link_cell(const xml_map_tree::cell_reference& ref, std::string_view val)
    {
        spreadsheet::iface::import_sheet* sheet = m_factory.get_sheet(ref.pos.sheet);
        if (sheet)
            sheet->set_auto(ref.pos.row, ref.pos.col, val);
    }

    void set_field_link_cell(xml_map_tree::field_in_range& field, std::string_view val)
    {
        assert(field.ref);
        assert(!field.ref->pos.sheet.empty());

        const xml_map_tree::cell_position& pos = field.ref->pos;
        spreadsheet::iface::import_sheet* sheet = m_factory.get_sheet(pos.sheet);
        if (sheet)
            sheet->set_auto(
               pos.row + field.ref->row_position,
               pos.col + field.column_pos,
               val);
    }

public:
    xml_data_sax_handler(
        spreadsheet::iface::import_factory& factory,
        xml_map_tree::const_element_list_type& link_positions,
        const xml_map_tree& map_tree) :
        m_factory(factory),
        m_link_positions(link_positions),
        m_map_tree(map_tree),
        m_map_tree_walker(map_tree.get_tree_walker()),
        mp_current_elem(nullptr),
        m_in_range_ref(false),
        mp_increment_row(nullptr) {}

    void doctype(const sax::doctype_declaration&)
    {
    }

    void start_declaration(std::string_view)
    {
    }

    void end_declaration(std::string_view)
    {
        m_attrs.clear();
    }

    void start_element(const sax_ns_parser_element& elem)
    {
        m_scopes.emplace_back(elem.ns, elem.name);
        scope& cur = m_scopes.back();
        cur.element_open_begin = elem.begin_pos;
        cur.element_open_end = elem.end_pos;
        m_current_chars = std::string_view{};

        mp_current_elem = m_map_tree_walker.push_element({elem.ns, elem.name});
        if (mp_current_elem)
        {
            if (mp_current_elem->row_group && mp_increment_row == mp_current_elem->row_group)
            {
                // The last closing element was a row group boundary.  Increment the row position.
                xml_map_tree::range_reference* ref = mp_current_elem->row_group;
                ++ref->row_position;
                mp_increment_row = nullptr;
            }

            // Go through all linked attributes that belong to this element,
            // and see if they exist in this content xml.
            for (const auto& p_attr : mp_current_elem->attributes)
            {
                const xml_map_tree::attribute& linked_attr = *p_attr;
                const sax_ns_parser_attribute* p = find_attr_by_name(linked_attr.name);
                if (!p)
                    continue;

                // This attribute is linked. Import its value.

                std::string_view val_trimmed = trim(p->value);
                switch (linked_attr.ref_type)
                {
                    case xml_map_tree::reference_type::cell:
                        set_single_link_cell(*linked_attr.cell_ref, val_trimmed);
                        break;
                    case xml_map_tree::reference_type::range_field:
                    {
                        set_field_link_cell(*linked_attr.field_ref, val_trimmed);
                        break;
                    }
                    default:
                        ;
                }

                // Record the namespace alias used in the content stream.
                linked_attr.ns_alias = m_map_tree.intern_string(p->ns_alias);
            }

            if (mp_current_elem->range_parent)
                m_in_range_ref = true;
        }
        m_attrs.clear();
    }

    void end_element(const sax_ns_parser_element& elem)
    {
        assert(!m_scopes.empty());

        if (mp_current_elem)
        {
            switch (mp_current_elem->ref_type)
            {
                case xml_map_tree::reference_type::cell:
                {
                    set_single_link_cell(*mp_current_elem->cell_ref, m_current_chars);
                    break;
                }
                case xml_map_tree::reference_type::range_field:
                {
                    set_field_link_cell(*mp_current_elem->field_ref, m_current_chars);
                    break;
                }
                default:
                    ;
            }

            if (mp_current_elem->row_group)
            {
                // This element defines a row-group boundary.
                spreadsheet::row_t row_start = mp_current_elem->row_group_position;
                spreadsheet::row_t row_end = mp_current_elem->row_group->row_position - 1;
                if (row_end > row_start)
                {
                    // This is the end of a parent row-group.  Fill down the
                    // cell values.
                    const xml_map_tree::range_reference& ref = *mp_current_elem->row_group;

                    spreadsheet::iface::import_sheet* sheet = m_factory.get_sheet(ref.pos.sheet);

                    if (sheet)
                    {
                        row_start += ref.pos.row + 1;
                        row_end += ref.pos.row + 1;

                        for (spreadsheet::col_t col : mp_current_elem->linked_range_fields)
                        {
                            col += ref.pos.col;
                            sheet->fill_down_cells(row_start, col, row_end - row_start);
                        }
                    }
                }

                mp_current_elem->row_group_position = mp_current_elem->row_group->row_position;
                mp_increment_row = mp_current_elem->row_group;
            }

            // Store the end element position in stream for linked elements.
            const scope& cur = m_scopes.back();
            if (mp_current_elem->ref_type == xml_map_tree::reference_type::cell ||
                mp_current_elem->range_parent ||
                (!m_in_range_ref && mp_current_elem->unlinked_attribute_anchor()))
            {
                // either single link element, parent of range link elements,
                // or an unlinked attribute anchor outside linked ranges.
                mp_current_elem->stream_pos.open_begin = cur.element_open_begin;
                mp_current_elem->stream_pos.open_end = cur.element_open_end;
                mp_current_elem->stream_pos.close_begin = elem.begin_pos;
                mp_current_elem->stream_pos.close_end = elem.end_pos;
                m_link_positions.push_back(mp_current_elem);
            }

            if (mp_current_elem->range_parent)
                m_in_range_ref = false;

            // Record the namespace alias used in the content stream.
            mp_current_elem->ns_alias = m_map_tree.intern_string(elem.ns_alias);
        }

        m_scopes.pop_back();
        mp_current_elem = m_map_tree_walker.pop_element({elem.ns, elem.name});
    }

    void characters(std::string_view val, bool transient)
    {
        if (!mp_current_elem)
            return;

        m_current_chars = trim(val);
        if (transient)
            m_current_chars = m_pool.intern(m_current_chars).first;
    }

    void attribute(std::string_view name, std::string_view val)
    {
        if (name == "encoding")
        {
            if (auto* gs = m_factory.get_global_settings(); gs)
            {
                character_set_t cs = to_character_set(val);
                gs->set_character_set(cs);
            }
        }
    }

    void attribute(const sax_ns_parser_attribute& at)
    {
        m_attrs.push_back(at);
    }
};

/**
 * Used in write_range_reference_group().
 */
struct scope
{
    const xml_map_tree::element& element;
    xml_map_tree::element_store_type::const_iterator current_child_pos;
    xml_map_tree::element_store_type::const_iterator end_child_pos;
    bool opened:1;

    scope(const scope&) = delete;
    scope& operator=(const scope&) = delete;

    scope(const xml_map_tree::element& _elem) :
        element(_elem), opened(false)
    {
        current_child_pos = end_child_pos;

        if (element.elem_type == xml_map_tree::element_type::unlinked)
        {
            current_child_pos = element.child_elements->begin();
            end_child_pos = element.child_elements->end();
        }
    }
};

typedef std::vector<std::unique_ptr<scope>> scopes_type;

void write_opening_element(
    std::ostream& os, const xml_map_tree::element& elem, const xml_map_tree::range_reference& ref,
    const spreadsheet::iface::export_sheet& sheet, spreadsheet::row_t current_row, bool self_close)
{
    if (elem.attributes.empty())
    {
        // This element has no linked attributes. Just write the element name and be done with it.
        os << '<' << elem << '>';
        return;
    }

    // Element has one or more linked attributes.

    os << '<' << elem;

    for (const auto& p_attr : elem.attributes)
    {
        const xml_map_tree::attribute& attr = *p_attr;
        if (attr.ref_type != xml_map_tree::reference_type::range_field)
            // In theory this should never happen but it won't hurt to check.
            continue;

        os << ' ' << attr << "=\"";
        sheet.write_string(os, ref.pos.row + 1 + current_row, ref.pos.col + attr.field_ref->column_pos);
        os << "\"";
    }

    if (self_close)
        os << '/';

    os << '>';
}

void write_opening_element(
    std::ostream& os, const xml_map_tree::element& elem, const spreadsheet::iface::export_factory& fact, bool self_close)
{
    os << '<' << elem;
    for (const auto& p_attr : elem.attributes)
    {
        const xml_map_tree::attribute& attr = *p_attr;
        if (attr.ref_type != xml_map_tree::reference_type::cell)
            // We should only see single linked cell here, as all
            // field links are handled by the range parent above.
            continue;

        const xml_map_tree::cell_position& pos = attr.cell_ref->pos;

        const spreadsheet::iface::export_sheet* sheet = fact.get_sheet(pos.sheet);
        if (!sheet)
            continue;

        os << ' ' << attr << "=\"";
        sheet->write_string(os, pos.row, pos.col);
        os << "\"";
    }

    if (self_close)
        os << '/';

    os << '>';
}

/**
 * Write to the output stream a single range reference.
 *
 * @param os output stream.
 * @param root root map tree element representing the root of a single range
 *             reference.
 * @param ref range reference data.
 * @param factory export factory instance.
 */
void write_range_reference_group(
   std::ostream& os, const xml_map_tree::element& root, const xml_map_tree::range_reference& ref,
   const spreadsheet::iface::export_factory& factory)
{
    const spreadsheet::iface::export_sheet* sheet = factory.get_sheet(ref.pos.sheet);
    if (!sheet)
        return;

    scopes_type scopes;
    for (spreadsheet::row_t current_row = 0; current_row < ref.row_position; ++current_row)
    {
        scopes.push_back(std::make_unique<scope>(root)); // root element

        while (!scopes.empty())
        {
            bool new_scope = false;

            scope& cur_scope = *scopes.back();

            // Self-closing element has no child elements nor content.
            bool self_close =
                (cur_scope.current_child_pos == cur_scope.end_child_pos) &&
                (cur_scope.element.ref_type != xml_map_tree::reference_type::range_field);

            if (!cur_scope.opened)
            {
                // Write opening element of this scope only on the 1st entrance.
                write_opening_element(os, cur_scope.element, ref, *sheet, current_row, self_close);
                cur_scope.opened = true;
            }

            if (self_close)
            {
                scopes.pop_back();
                continue;
            }

            // Go though all child elements.
            for (; cur_scope.current_child_pos != cur_scope.end_child_pos; ++cur_scope.current_child_pos)
            {
                const xml_map_tree::element& child_elem = **cur_scope.current_child_pos;
                if (child_elem.elem_type == xml_map_tree::element_type::unlinked)
                {
                    // This is a non-leaf element.  Push a new scope with this
                    // element and re-start the loop.
                    ++cur_scope.current_child_pos;
                    scopes.push_back(std::make_unique<scope>(child_elem));
                    new_scope = true;
                    break;
                }

                // This is a leaf element.  This must be a field link element.
                if (child_elem.ref_type == xml_map_tree::reference_type::range_field)
                {
                    write_opening_element(os, child_elem, ref, *sheet, current_row, false);
                    sheet->write_string(os, ref.pos.row + 1 + current_row, ref.pos.col + child_elem.field_ref->column_pos);
                    os << "</" << child_elem << ">";
                }
            }

            if (new_scope)
                // Re-start the loop with a new scope.
                continue;

            // Write content of this element before closing it (if it's linked).
            if (scopes.back()->element.ref_type == xml_map_tree::reference_type::range_field)
                sheet->write_string(
                    os, ref.pos.row + 1 + current_row, ref.pos.col + scopes.back()->element.field_ref->column_pos);

            // Close this element for good, and exit the current scope.
            os << "</" << scopes.back()->element << ">";
            scopes.pop_back();
        }
    }
}

/**
 * Write to an output stream the sub-structure comprising one or more range
 * references.
 *
 * @param os output stream
 * @param elem_top topmost element in the range reference sub-structure.
 */
void write_range_reference(std::ostream& os, const xml_map_tree::element& elem_top, const spreadsheet::iface::export_factory& factory)
{
    // Top element is expected to have one or more child elements, and each
    // child element represents a separate database range.
    if (elem_top.elem_type != xml_map_tree::element_type::unlinked)
        return;

    assert(elem_top.child_elements);

    if (elem_top.child_elements->empty())
        return;

    // TODO: For now, we assume that there is only one child element under the
    // range ref parent.
    write_range_reference_group(
       os, **elem_top.child_elements->begin(), *elem_top.range_parent, factory);
}

struct less_by_opening_elem_pos
{
    bool operator() (const xml_map_tree::element* left, const xml_map_tree::element* right) const
    {
        return left->stream_pos.open_begin < right->stream_pos.open_begin;
    }
};

} // anonymous namespace

orcus_xml::orcus_xml(xmlns_repository& ns_repo, spreadsheet::iface::import_factory* im_fact, spreadsheet::iface::export_factory* ex_fact) :
    mp_impl(std::make_unique<impl>(ns_repo))
{
    mp_impl->im_factory = im_fact;
    mp_impl->ex_factory = ex_fact;
}

orcus_xml::~orcus_xml() {}

void orcus_xml::set_namespace_alias(std::string_view alias, std::string_view uri, bool default_ns)
{
    mp_impl->map_tree.set_namespace_alias(alias, uri, default_ns);
}

void orcus_xml::set_cell_link(std::string_view xpath, std::string_view sheet, spreadsheet::row_t row, spreadsheet::col_t col)
{
    std::string_view sheet_safe = mp_impl->map_tree.intern_string(sheet);
    mp_impl->map_tree.set_cell_link(xpath, xml_map_tree::cell_position(sheet_safe, row, col));
}

void orcus_xml::start_range(std::string_view sheet, spreadsheet::row_t row, spreadsheet::col_t col)
{
    std::string_view sheet_safe = mp_impl->map_tree.intern_string(sheet);
    mp_impl->cur_range_ref = xml_map_tree::cell_position(sheet_safe, row, col);
    mp_impl->map_tree.start_range(mp_impl->cur_range_ref);
}

void orcus_xml::append_field_link(std::string_view xpath, std::string_view label)
{
    mp_impl->map_tree.append_range_field_link(xpath, label);
}

void orcus_xml::set_range_row_group(std::string_view xpath)
{
    mp_impl->map_tree.set_range_row_group(xpath);
}

void orcus_xml::commit_range()
{
    mp_impl->cur_range_ref = xml_map_tree::cell_position();
    mp_impl->map_tree.commit_range();
}

void orcus_xml::append_sheet(std::string_view name)
{
    if (name.empty())
        return;

    mp_impl->im_factory->append_sheet(mp_impl->sheet_count++, name);
}

void orcus_xml::read_stream(std::string_view stream)
{
    if (stream.empty())
        return;

    // Insert the range headers and reset the row size counters.
    xml_map_tree::range_ref_map_type& range_refs = mp_impl->map_tree.get_range_references();

    for (const auto& ref_pair : range_refs)
    {
        const xml_map_tree::cell_position& ref = ref_pair.first;
        xml_map_tree::range_reference& range_ref = *ref_pair.second;
        range_ref.row_position = 1; // Reset the row offset.

        spreadsheet::iface::import_sheet* sheet = mp_impl->im_factory->get_sheet(ref.sheet);

        if (!sheet)
            continue;

        spreadsheet::row_t row = ref.row;
        spreadsheet::col_t col = ref.col;

        for (const xml_map_tree::linkable* e : range_ref.field_nodes)
        {
            if (e->label.empty())
            {
                // No custom header label. Create a label from the name of the linkable.
                std::string s = e->name.to_string(mp_impl->ns_repo);
                if (!s.empty())
                    sheet->set_auto(row, col, s);
            }
            else
                sheet->set_auto(row, col, e->label);

            ++col;
        }
    }

    // Parse the content xml.
    xmlns_context ns_cxt = mp_impl->ns_repo.create_context(); // new ns context for the content xml stream.
    xml_data_sax_handler handler(
       *mp_impl->im_factory, mp_impl->link_positions, mp_impl->map_tree);

    sax_ns_parser<xml_data_sax_handler> parser(stream, ns_cxt, handler);
    parser.parse();
}

#if ORCUS_DEBUG_XML

namespace {

void dump_links(const xml_map_tree::const_element_list_type& links)
{
    cout << "link count: " << links.size() << endl;
}

}

#endif

void orcus_xml::write(std::string_view stream, std::ostream& out) const
{
    if (!mp_impl->ex_factory)
        // We can't export data witout export factory.
        return;

    if (stream.empty())
        // Source input stream is empty.
        return;

    xml_map_tree::const_element_list_type& links = mp_impl->link_positions;
    if (links.empty())
        // nothing to write.
        return;

    // Sort all link position by opening element positions.
    std::sort(links.begin(), links.end(), less_by_opening_elem_pos());

    spreadsheet::iface::export_factory& fact = *mp_impl->ex_factory;
    xml_map_tree::const_element_list_type::const_iterator it = links.begin(), it_end = links.end();

#if ORCUS_DEBUG_XML
    dump_links(links);
#endif

    const char* p0 = stream.data();
    std::ptrdiff_t begin_pos = 0;

    for (; it != it_end; ++it)
    {
        const xml_map_tree::element& elem = **it;
        if (elem.ref_type == xml_map_tree::reference_type::cell)
        {
            // Single cell link
            const xml_map_tree::cell_position& pos = elem.cell_ref->pos;

            const spreadsheet::iface::export_sheet* sheet = fact.get_sheet(pos.sheet);
            if (!sheet)
                continue;

            std::ptrdiff_t open_begin  = elem.stream_pos.open_begin;
            std::ptrdiff_t close_begin = elem.stream_pos.close_begin;
            std::ptrdiff_t close_end   = elem.stream_pos.close_end;

            assert(open_begin > begin_pos);
            out << std::string_view(p0+begin_pos, open_begin-begin_pos); // stream since last linked element.

            write_opening_element(out, elem, fact, false);
            sheet->write_string(out, pos.row, pos.col);
            out << std::string_view(p0+close_begin, close_end-close_begin); // closing element.
            begin_pos = close_end;
        }
        else if (elem.range_parent)
        {
            // Range link
            const xml_map_tree::range_reference& ref = *elem.range_parent;
            const xml_map_tree::cell_position& pos = ref.pos;

            const spreadsheet::iface::export_sheet* sheet = fact.get_sheet(pos.sheet);
            if (!sheet)
                continue;

            std::ptrdiff_t open_begin  = elem.stream_pos.open_begin;
            std::ptrdiff_t close_begin = elem.stream_pos.close_begin;
            std::ptrdiff_t close_end   = elem.stream_pos.close_end;

            assert(open_begin > begin_pos);
            out << std::string_view(p0+begin_pos, open_begin-begin_pos); // stream since last linked element.

            write_opening_element(out, elem, fact, false);
            write_range_reference(out, elem, fact);
            out << std::string_view(p0+close_begin, close_end-close_begin); // closing element.
            begin_pos = close_end;
        }
        else if (elem.unlinked_attribute_anchor())
        {
            // Element is not linked but has one or more attributes that are
            // linked.  Here, only write the opening element with attributes.

            std::ptrdiff_t open_begin = elem.stream_pos.open_begin;
            std::ptrdiff_t open_end   = elem.stream_pos.open_end;

            bool self_close = elem.stream_pos.open_begin == elem.stream_pos.close_begin;

            assert(open_begin > begin_pos);
            out << std::string_view(p0+begin_pos, open_begin-begin_pos); // stream since last linked element.

            write_opening_element(out, elem, fact, self_close);
            begin_pos = open_end;
        }
        else
            throw general_error("Non-link element type encountered.");
    }

    // Flush the remaining stream.
    out << std::string_view(p0+begin_pos, stream.size()-begin_pos);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
