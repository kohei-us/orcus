/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_CELL_BUFFER_HPP__
#define __ORCUS_CELL_BUFFER_HPP__

#include "env.hpp"

#include <string>

namespace orcus {

/**
 * Temporary cell buffer used to decode encoded cell values.  This is used in
 * the sax, json and csv parsers.
 */
class ORCUS_PSR_DLLPUBLIC cell_buffer
{
    std::string m_buffer;
    size_t m_buf_size;
public:
    cell_buffer(const cell_buffer&) = delete;

    cell_buffer();
    ~cell_buffer();

    void append(const char* p, size_t len);
    void reset();
    const char* get() const;

    /**
     * Get the logical size of the buffer.  This may differ from the actual
     * buffer size.
     *
     * @return logical size of the buffer.
     */
    size_t size() const;
    bool empty() const;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
