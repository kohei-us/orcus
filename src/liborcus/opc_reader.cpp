/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opc_reader.hpp"
#include "xml_stream_parser.hpp"

#include "ooxml_global.hpp"
#include "opc_context.hpp"
#include "ooxml_tokens.hpp"

#include "orcus/config.hpp"

#include <iostream>

namespace orcus {

namespace {

class print_xml_content_types
{
public:
    print_xml_content_types(const char* prefix) :
        m_prefix(prefix) {}

    void operator() (const xml_part_t& v) const
    {
        std::cout << "* " << m_prefix << ": " << v.first;
        if (v.second)
            std::cout << " (" << v.second << ")";
        else
            std::cout << " (<unknown content type>)";
        std::cout << std::endl;
    }
private:
    const char* m_prefix;
};

}

opc_reader::part_handler::~part_handler() {}

opc_reader::opc_reader(const config& opt, xmlns_repository& ns_repo, session_context& cxt, part_handler& handler) :
    m_config(opt),
    m_ns_repo(ns_repo),
    m_session_cxt(cxt),
    m_handler(handler),
    m_opc_rel_handler(m_session_cxt, opc_tokens, std::make_unique<opc_relations_context>(m_session_cxt, opc_tokens)) {}

void opc_reader::read_file(std::unique_ptr<zip_archive_stream>&& stream)
{
    m_archive_stream.reset(stream.release());
    m_archive.reset(new zip_archive(m_archive_stream.get()));

    m_archive->load();

    m_dir_stack.push_back(std::string()); // push root directory.

    if (m_config.debug)
        list_content();
    read_content();

    m_archive.reset();
    m_archive_stream.reset();
}

bool opc_reader::open_zip_stream(const std::string& path, std::vector<unsigned char>& buf)
{
    try
    {
        std::vector<unsigned char> entry = m_archive->read_file_entry(path.c_str());
        buf.swap(entry);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void opc_reader::read_part(std::string_view path, const schema_t type, opc_rel_extra* data)
{
    assert(!m_dir_stack.empty());

    dir_stack_type dir_changed;

    // Change current directory and read the in-file.
    const char* p = path.data();
    const char* p_name = nullptr;
    size_t name_len = 0;
    for (size_t i = 0, n = path.size(); i < n; ++i, ++p)
    {
        if (!p_name)
            p_name = p;

        ++name_len;

        if (*p == '/')
        {
            // Push a new directory.
            std::string dir_name(p_name, name_len);
            if (dir_name == "..")
            {
                dir_changed.push_back(m_dir_stack.back());
                m_dir_stack.pop_back();
            }
            else
            {
                m_dir_stack.push_back(dir_name);

                // Add a null directory to the change record to remove it at the end.
                dir_changed.push_back(std::string());
            }

            p_name = nullptr;
            name_len = 0;
        }
    }

    if (p_name)
    {
        // This is a file.
        std::string file_name(p_name, name_len);
        std::string cur_dir = get_current_dir();
        std::string full_path = resolve_file_path(cur_dir, file_name);

        if (m_handled_parts.count(full_path) > 0)
        {
            // This part has been previously read.  Let's not read it twice.
            if (m_config.debug)
            {
                std::cout << "---" << std::endl;
                std::cout << "skipping previously read part: " << full_path << std::endl;
            }
        }
        else if (m_handler.handle_part(type, cur_dir, file_name, data))
        {
            m_handled_parts.insert(full_path);
        }
        else if (m_config.debug)
        {
            std::cout << "---" << std::endl;
            std::cout << "unhandled relationship type: " << type << std::endl;
        }
    }

    // Unwind to the original directory.
    while (!dir_changed.empty())
    {
        const std::string& dir = dir_changed.back();
        if (dir.empty())
            // remove added directory.
            m_dir_stack.pop_back();
        else
            // re-add removed directory.
            m_dir_stack.push_back(dir);

        dir_changed.pop_back();
    }
}

void opc_reader::check_relation_part(
    const std::string& file_name, opc_rel_extras_t* extras, sort_compare_type* sorter)
{
    // Read the relationship file associated with this file, located at
    // _rels/<file name>.rels.
    std::vector<opc_rel_t> rels;
    m_dir_stack.push_back(std::string("_rels/"));
    std::string rels_file_name = file_name + ".rels";
    read_relations(rels_file_name.c_str(), rels);
    m_dir_stack.pop_back();

    if (sorter)
        std::sort(rels.begin(), rels.end(), *sorter);

    if (m_config.debug)
        std::for_each(rels.begin(), rels.end(), print_opc_rel());

    std::for_each(rels.begin(), rels.end(),
        [&](opc_rel_t& v)
        {
            opc_rel_extra* data = nullptr;
            if (extras)
            {
                // See if there is an extra data associated with this relation ID.
                opc_rel_extras_t::map_type::iterator it = extras->data.find(v.rid);
                if (it != extras->data.end())
                    // There is one !
                    data = it->second.get();
            }
            read_part(v.target, v.type, data);
        }
    );
}

void opc_reader::list_content() const
{
    size_t num = m_archive->get_file_entry_count();
    std::cout << "number of files this archive contains: " << num << std::endl;

    for (size_t i = 0; i < num; ++i)
    {
        std::string_view filename = m_archive->get_file_entry_name(i);
        std::cout << filename << std::endl;
    }
}

void opc_reader::read_content()
{
    if (m_dir_stack.empty())
        return;

    // [Content_Types].xml

    read_content_types();
    if (m_config.debug)
    {
        std::for_each(m_parts.begin(), m_parts.end(), print_xml_content_types("part name"));
        std::for_each(m_ext_defaults.begin(), m_ext_defaults.end(), print_xml_content_types("extension default"));
    }

    // _rels/.rels

    m_dir_stack.push_back(std::string("_rels/"));
    std::vector<opc_rel_t> rels;
    read_relations(".rels", rels);
    m_dir_stack.pop_back();

    if (m_config.debug)
        std::for_each(rels.begin(), rels.end(), print_opc_rel());

    std::for_each(rels.begin(), rels.end(),
        [this](opc_rel_t& v)
        {
            read_part(v.target, v.type, nullptr);
        }
    );
}

void opc_reader::read_content_types()
{
    std::string filepath("[Content_Types].xml");
    std::vector<unsigned char> buffer;
    if (!open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        m_config, m_ns_repo, opc_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xml_stream_handler>(
        m_session_cxt, opc_tokens,
        std::make_unique<opc_content_types_context>(m_session_cxt, opc_tokens));

    parser.set_handler(handler.get());
    parser.parse();

    auto& context = static_cast<opc_content_types_context&>(handler->get_root_context());
    context.pop_parts(m_parts);
    context.pop_ext_defaults(m_ext_defaults);
}

void opc_reader::read_relations(const char* path, std::vector<opc_rel_t>& rels)
{
    std::string filepath = resolve_file_path(get_current_dir(), path);
    if (m_config.debug)
        std::cout << "relation file path: " << filepath << std::endl;

    std::vector<unsigned char> buffer;
    if (!open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        m_config, m_ns_repo, opc_tokens,
        reinterpret_cast<const char*>(buffer.data()), buffer.size());

    auto& context = static_cast<opc_relations_context&>(m_opc_rel_handler.get_context());
    context.init();
    parser.set_handler(&m_opc_rel_handler);
    parser.parse();
    context.pop_rels(rels);
}

std::string opc_reader::get_current_dir() const
{
    std::ostringstream os;
    for (const auto& dir : m_dir_stack)
        os << dir;
    return os.str();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
