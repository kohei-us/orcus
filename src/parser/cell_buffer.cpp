/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/cell_buffer.hpp"

#include <cstring>

namespace orcus {

cell_buffer::cell_buffer() : m_buf_size(0) {}

cell_buffer::~cell_buffer() = default;

void cell_buffer::append(std::string_view s)
{
    if (s.empty())
        return;

    std::size_t size_needed = m_buf_size + s.size();
    if (m_buffer.size() < size_needed)
        m_buffer.resize(size_needed);

    char* p_dest = &m_buffer[m_buf_size];
    std::strncpy(p_dest, s.data(), s.size());
    m_buf_size += s.size();
}

void cell_buffer::reset()
{
    m_buf_size = 0;
}

std::string_view cell_buffer::str() const
{
    return std::string_view{m_buffer.data(), m_buf_size};
}

bool cell_buffer::empty() const
{
    return m_buf_size == 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
