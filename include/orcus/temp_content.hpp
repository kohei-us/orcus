/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2026 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#pragma once

#include "env.hpp"

#include <memory>
#include <string_view>

namespace orcus {

enum class temp_content_store_t
{
    /** Content is not stored. */
    uninitialized,
    /** Content is stored in dynamically-allocated memory. */
    heap_allocated,
    /** Content is stored in a memory-mapped temporary file. */
    memory_mapped
};

/**
 * Buffer for temporary content.
 *
 * The size of the buffer must be specified at construction, and cannot change
 * once constructed.  The content of the buffer is left uninitialized upon
 * construction.
 *
 * @note This class is movable but not copyable.
 */
class ORCUS_PSR_DLLPUBLIC temp_content
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    temp_content(const temp_content&) = delete;
    temp_content& operator=(const temp_content&) = delete;

    temp_content();
    temp_content(std::size_t buffer_size, temp_content_store_t store);
    temp_content(temp_content&& other) noexcept;
    ~temp_content();

    temp_content& operator=(temp_content&& other) noexcept;

    void swap(temp_content& other) noexcept;

    temp_content_store_t store_type() const noexcept;

    char* data() noexcept;

    const char* data() const noexcept;

    std::size_t size() const noexcept;

    bool empty() const noexcept;

    std::string_view str() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

