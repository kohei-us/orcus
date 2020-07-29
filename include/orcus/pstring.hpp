/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_PSTRING_HPP
#define INCLUDED_ORCUS_PSTRING_HPP

#include "orcus/env.hpp"

#include <cstdlib>
#include <string>
#include <cstring>
#include <ostream>

namespace orcus {

/**
 * This string class does not have its own char array buffer; it only stores
 * the memory position of the first char of an existing char array and its
 * size.  When using this class, it is important that the string object
 * being referenced by an instance of this class will stay valid during its
 * life time.
 */
class ORCUS_PSR_DLLPUBLIC pstring
{
    friend ::std::ostream& operator<< (::std::ostream& os, const pstring& str);

public:

    pstring() : m_pos(nullptr), m_size(0) {}
    pstring(const char* _pos);
    pstring(const char* _pos, size_t _size) : m_pos(_pos), m_size(_size) {}
    pstring(const std::string& s) : m_pos(s.data()), m_size(s.size()) {}

    ::std::string str() const { return ::std::string(m_pos, m_size); }

    size_t size() const { return m_size; }
    const char& operator[](size_t idx) const { return m_pos[idx]; }

    pstring& operator= (const pstring& r)
    {
        m_pos = r.m_pos;
        m_size = r.m_size;
        return *this;
    }

    const char* get() const { return m_pos; }

    const char* data() const { return m_pos; }

    bool operator== (const pstring& r) const;

    bool operator!= (const pstring& r) const
    {
        return !operator==(r);
    }

    bool operator< (const pstring& r) const;

    bool operator== (const char* _str) const;

    bool operator!= (const char* _str) const
    {
        return !operator==(_str);
    }

    pstring trim() const;

    bool empty() const { return m_size == 0; }

    void clear()
    {
        m_pos = nullptr;
        m_size = 0;
    }

    void resize(size_t new_size);

    struct ORCUS_PSR_DLLPUBLIC hash
    {
        size_t operator() (const pstring& val) const;
    };

private:
    const char* m_pos;
    size_t      m_size;
};

inline ::std::ostream& operator<< (::std::ostream& os, const pstring& str)
{
    return os.write(str.data(), str.size());
}

ORCUS_PSR_DLLPUBLIC std::string operator+ (const std::string& left, const pstring& right);
ORCUS_PSR_DLLPUBLIC std::string& operator+= (std::string& left, const pstring& right);

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
