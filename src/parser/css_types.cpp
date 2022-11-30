/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/css_types.hpp>
#include <mdds/sorted_string_map.hpp>
#include <mdds/global.hpp>

#include <sstream>

namespace orcus { namespace css {

const pseudo_element_t pseudo_element_after        = 0x0001;
const pseudo_element_t pseudo_element_before       = 0x0002;
const pseudo_element_t pseudo_element_first_letter = 0x0004;
const pseudo_element_t pseudo_element_first_line   = 0x0008;
const pseudo_element_t pseudo_element_selection    = 0x0010;
const pseudo_element_t pseudo_element_backdrop     = 0x0020;

namespace {

namespace pseudo_elem {

using map_type = mdds::sorted_string_map<pseudo_element_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "after",        pseudo_element_after        },
    { "backdrop",     pseudo_element_backdrop     },
    { "before",       pseudo_element_before       },
    { "first-letter", pseudo_element_first_letter },
    { "first-line",   pseudo_element_first_line   },
    { "selection",    pseudo_element_selection    },
};

const map_type& get()
{
    static map_type map(entries, std::size(entries), 0);
    return map;
}

} // namespace pseudo_elem

}

pseudo_element_t to_pseudo_element(std::string_view s)
{
    return pseudo_elem::get().find(s);
}

const pseudo_class_t pseudo_class_active            = 0x0000000000000001;
const pseudo_class_t pseudo_class_checked           = 0x0000000000000002;
const pseudo_class_t pseudo_class_default           = 0x0000000000000004;
const pseudo_class_t pseudo_class_dir               = 0x0000000000000008;
const pseudo_class_t pseudo_class_disabled          = 0x0000000000000010;
const pseudo_class_t pseudo_class_empty             = 0x0000000000000020;
const pseudo_class_t pseudo_class_enabled           = 0x0000000000000040;
const pseudo_class_t pseudo_class_first             = 0x0000000000000080;
const pseudo_class_t pseudo_class_first_child       = 0x0000000000000100;
const pseudo_class_t pseudo_class_first_of_type     = 0x0000000000000200;
const pseudo_class_t pseudo_class_fullscreen        = 0x0000000000000400;
const pseudo_class_t pseudo_class_focus             = 0x0000000000000800;
const pseudo_class_t pseudo_class_hover             = 0x0000000000001000;
const pseudo_class_t pseudo_class_indeterminate     = 0x0000000000002000;
const pseudo_class_t pseudo_class_in_range          = 0x0000000000004000;
const pseudo_class_t pseudo_class_invalid           = 0x0000000000008000;
const pseudo_class_t pseudo_class_lang              = 0x0000000000010000;
const pseudo_class_t pseudo_class_last_child        = 0x0000000000020000;
const pseudo_class_t pseudo_class_last_of_type      = 0x0000000000040000;
const pseudo_class_t pseudo_class_left              = 0x0000000000080000;
const pseudo_class_t pseudo_class_link              = 0x0000000000100000;
const pseudo_class_t pseudo_class_not               = 0x0000000000200000;
const pseudo_class_t pseudo_class_nth_child         = 0x0000000000400000;
const pseudo_class_t pseudo_class_nth_last_child    = 0x0000000000800000;
const pseudo_class_t pseudo_class_nth_last_of_type  = 0x0000000001000000;
const pseudo_class_t pseudo_class_nth_of_type       = 0x0000000002000000;
const pseudo_class_t pseudo_class_only_child        = 0x0000000004000000;
const pseudo_class_t pseudo_class_only_of_type      = 0x0000000008000000;
const pseudo_class_t pseudo_class_optional          = 0x0000000010000000;
const pseudo_class_t pseudo_class_out_of_range      = 0x0000000020000000;
const pseudo_class_t pseudo_class_read_only         = 0x0000000040000000;
const pseudo_class_t pseudo_class_read_write        = 0x0000000080000000;
const pseudo_class_t pseudo_class_required          = 0x0000000100000000;
const pseudo_class_t pseudo_class_right             = 0x0000000200000000;
const pseudo_class_t pseudo_class_root              = 0x0000000400000000;
const pseudo_class_t pseudo_class_scope             = 0x0000000800000000;
const pseudo_class_t pseudo_class_target            = 0x0000001000000000;
const pseudo_class_t pseudo_class_valid             = 0x0000002000000000;
const pseudo_class_t pseudo_class_visited           = 0x0000004000000000;

namespace {

namespace pseudo_class {

using map_type = mdds::sorted_string_map<pseudo_class_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "active",           pseudo_class_active           },
    { "checked",          pseudo_class_checked          },
    { "default",          pseudo_class_default          },
    { "dir",              pseudo_class_dir              },
    { "disabled",         pseudo_class_disabled         },
    { "empty",            pseudo_class_empty            },
    { "enabled",          pseudo_class_enabled          },
    { "first",            pseudo_class_first            },
    { "first-child",      pseudo_class_first_child      },
    { "first-of-type",    pseudo_class_first_of_type    },
    { "focus",            pseudo_class_focus            },
    { "fullscreen",       pseudo_class_fullscreen       },
    { "hover",            pseudo_class_hover            },
    { "in-range",         pseudo_class_in_range         },
    { "indeterminate",    pseudo_class_indeterminate    },
    { "invalid",          pseudo_class_invalid          },
    { "lang",             pseudo_class_lang             },
    { "last-child",       pseudo_class_last_child       },
    { "last-of-type",     pseudo_class_last_of_type     },
    { "left",             pseudo_class_left             },
    { "link",             pseudo_class_link             },
    { "not",              pseudo_class_not              },
    { "nth-child",        pseudo_class_nth_child        },
    { "nth-last-child",   pseudo_class_nth_last_child   },
    { "nth-last-of-type", pseudo_class_nth_last_of_type },
    { "nth-of-type",      pseudo_class_nth_of_type      },
    { "only-child",       pseudo_class_only_child       },
    { "only-of-type",     pseudo_class_only_of_type     },
    { "optional",         pseudo_class_optional         },
    { "out-of-range",     pseudo_class_out_of_range     },
    { "read-only",        pseudo_class_read_only        },
    { "read-write",       pseudo_class_read_write       },
    { "required",         pseudo_class_required         },
    { "right",            pseudo_class_right            },
    { "root",             pseudo_class_root             },
    { "scope",            pseudo_class_scope            },
    { "target",           pseudo_class_target           },
    { "valid",            pseudo_class_valid            },
    { "visited",          pseudo_class_visited          },
};

const map_type& get()
{
    static map_type map(entries, std::size(entries), 0);
    return map;
}

} // namespace pseudo_class

}

pseudo_class_t to_pseudo_class(std::string_view s)
{
    return pseudo_class::get().find(s);
}

std::string pseudo_class_to_string(pseudo_class_t val)
{
    std::ostringstream os;
    std::size_t n = std::size(pseudo_class::entries);
    const pseudo_class::map_type::entry* p = pseudo_class::entries;
    const pseudo_class::map_type::entry* p_end = p + n;
    for (; p != p_end; ++p)
    {
        if (val & p->value)
            os << ":" << p->key;
    }

    return os.str();
}

namespace {

typedef mdds::sorted_string_map<property_function_t> propfunc_map_type;

// Keys must be sorted.
propfunc_map_type::entry propfunc_type_entries[] = {
    { MDDS_ASCII("hsl"),  property_function_t::hsl  },
    { MDDS_ASCII("hsla"), property_function_t::hsla },
    { MDDS_ASCII("rgb"),  property_function_t::rgb  },
    { MDDS_ASCII("rgba"), property_function_t::rgba },
    { MDDS_ASCII("url"),  property_function_t::url  }
};

}

property_function_t to_property_function(std::string_view s)
{
    static propfunc_map_type propfunc_map(
        propfunc_type_entries, std::size(propfunc_type_entries), property_function_t::unknown);

    return propfunc_map.find(s.data(), s.size());
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
