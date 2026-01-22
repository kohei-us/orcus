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

#include <orcus/temp_content.hpp>
#include <orcus/stream.hpp>

#include "filesystem_env.hpp"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <fstream>
#include <vector>
#include <variant>

namespace bip = boost::interprocess;

namespace orcus {

namespace {

struct mmap_store
{
    fs::path temp_path;

    boost::uintmax_t content_size;
    bip::file_mapping mapped_file;
    bip::mapped_region mapped_region;

    mmap_store(std::size_t file_size) :
        temp_path(fs::temp_directory_path() / generate_uuid4()),
        content_size(file_size)
    {
        {
            std::ofstream ofs{temp_path, std::ios::binary};
            if (!ofs)
                throw std::runtime_error{"failed to create a temp file!"};
        }

        fs::resize_file(temp_path, file_size);

        mapped_file = bip::file_mapping(temp_path.c_str(), bip::read_write);
        mapped_region = bip::mapped_region(mapped_file, bip::read_write, 0, content_size);
    }

    ~mmap_store()
    {
        fs::remove(temp_path);
    }
};

using backend_store_type = std::variant<std::monostate, std::unique_ptr<mmap_store>, std::vector<char>>;

} // anonymous namespace

struct temp_content::impl
{
    backend_store_type store;

    std::size_t content_size = 0;
    char* content = nullptr;

    impl()
    {
    }

    impl(std::size_t file_size, temp_content_store_t st) :
        content_size(file_size)
    {
        if (!file_size)
            return;

        switch (st)
        {
            case temp_content_store_t::heap_allocated:
            {
                std::vector<char> this_store(file_size);
                content = this_store.data();
                store = std::move(this_store);
                break;
            }
            case temp_content_store_t::memory_mapped:
            {
                auto this_store = std::make_unique<mmap_store>(file_size);
                content = static_cast<char*>(this_store->mapped_region.get_address());
                store = std::move(this_store);
                break;
            }
            case temp_content_store_t::uninitialized:
            {
                throw std::invalid_argument{"temp_content cannot be constructed with 'uninitialized' store type"};
            }
        }
    }
};

temp_content::temp_content() : mp_impl(std::make_unique<impl>())
{
}

temp_content::temp_content(std::size_t file_size, temp_content_store_t store) :
    mp_impl(std::make_unique<impl>(file_size, store))
{
}

temp_content::temp_content(temp_content&& other) noexcept :
    mp_impl(std::move(other.mp_impl))
{
}

temp_content::~temp_content() = default;

temp_content& temp_content::operator=(temp_content&& other) noexcept
{
    temp_content temp(std::move(other));
    temp.swap(*this);

    return *this;
}

void temp_content::swap(temp_content& other) noexcept
{
    mp_impl.swap(other.mp_impl);
}

temp_content_store_t temp_content::store_type() const noexcept
{
    switch (mp_impl->store.index())
    {
        case 0:
            break;
        case 1:
            return temp_content_store_t::memory_mapped;
        case 2:
            return temp_content_store_t::heap_allocated;
    }

    return temp_content_store_t::uninitialized;
}

char* temp_content::data() noexcept
{
    return mp_impl->content;
}

const char* temp_content::data() const noexcept
{
    return mp_impl->content;
}

std::size_t temp_content::size() const noexcept
{
    return mp_impl->content_size;
}

bool temp_content::empty() const noexcept
{
    return mp_impl->content == nullptr;
}

std::string_view temp_content::str() const
{
    return std::string_view{mp_impl->content, mp_impl->content_size};
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

