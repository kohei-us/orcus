/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "env.hpp"

#include <memory>
#include <string_view>

namespace orcus {

enum class unnamed_buffer_store_t
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
class ORCUS_PSR_DLLPUBLIC unnamed_buffer
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    unnamed_buffer(const unnamed_buffer&) = delete;
    unnamed_buffer& operator=(const unnamed_buffer&) = delete;

    unnamed_buffer();
    unnamed_buffer(std::size_t buffer_size, unnamed_buffer_store_t store);
    unnamed_buffer(unnamed_buffer&& other) noexcept;
    ~unnamed_buffer();

    unnamed_buffer& operator=(unnamed_buffer&& other) noexcept;

    void swap(unnamed_buffer& other) noexcept;

    unnamed_buffer_store_t store_type() const noexcept;

    char* data() noexcept;

    const char* data() const noexcept;

    std::size_t size() const noexcept;

    bool empty() const noexcept;

    std::string_view str() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
