/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/sax_ns_parser.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/base64.hpp>

#include "mso/encryption_info.hpp"

#define ORCUS_DEBUG_MSO_ENCRYPTION_INFO 1

#if ORCUS_DEBUG_MSO_ENCRYPTION_INFO
#include <iostream>
#endif

#include <vector>
#include <ostream>

namespace orcus { namespace mso {

namespace {

const xmlns_id_t NS_mso_encryption = "http://schemas.microsoft.com/office/2006/encryption";
const xmlns_id_t NS_mso_password = "http://schemas.microsoft.com/office/2006/keyEncryptor/password";

const xmlns_id_t NS_mso_all[] = {
    NS_mso_encryption,
    NS_mso_password,
    nullptr
};

class char_printer
{
    std::ostream& m_os;
public:
    char_printer(std::ostream& os) : m_os(os) {}
    void operator() (const char c) const
    {
        short v = c;
        v &= 0x00FF;
        m_os << std::hex << std::uppercase;
        if (v < 16)
            m_os << '0';
        m_os << v << ' ';
    }
};

void print_base64(const char* caption, std::string_view base64)
{
    std::cout << caption << " (base64): " << base64 << std::endl;
    std::vector<uint8_t> value = orcus::decode_from_base64(base64);
    std::cout << caption << " (binary): ";
    std::for_each(value.begin(), value.end(), char_printer(std::cout));
    std::cout << std::endl;
}

class key_data_attr_handler
{
public:
    void operator() (const sax_ns_parser_attribute& attr)
    {
        if (attr.ns != NS_mso_encryption)
            // wrong namespace
            return;

        if (attr.name == "saltSize")
            std::cout << "salt size: " << attr.value << std::endl;
        else if (attr.name == "blockSize")
            std::cout << "block size: " << attr.value << std::endl;
        else if (attr.name == "keyBits")
            std::cout << "key bits: " << attr.value << std::endl;
        else if (attr.name == "hashSize")
            std::cout << "hash size: " << attr.value << std::endl;
        else if (attr.name == "cipherAlgorithm")
            std::cout << "cipher algorithm: " << attr.value << std::endl;
        else if (attr.name == "cipherChaining")
            std::cout << "cipher chaining: " << attr.value << std::endl;
        else if (attr.name == "hashAlgorithm")
            std::cout << "hash algorithm: " << attr.value << std::endl;
        else if (attr.name == "saltValue")
            print_base64("salt value", attr.value);
    }
};

class data_integrity_attr_handler
{
public:
    void operator() (const sax_ns_parser_attribute& attr)
    {
        if (attr.ns != NS_mso_encryption)
            // wrong namespace
            return;

        if (attr.name == "encryptedHmacKey")
            print_base64("encrypted HMAC key", attr.value);
        else if (attr.name == "encryptedHmacValue")
            print_base64("encrypted HMAC value", attr.value);
    }
};

class password_encrypted_key_attr_handler
{
public:
    void operator() (const sax_ns_parser_attribute& attr)
    {
        if (attr.ns != NS_mso_encryption)
            // wrong namespace
            return;

        if (attr.name == "spinCount")
            std::cout << "spin count: " << attr.value << std::endl;
        else if (attr.name == "saltSize")
            std::cout << "salt size: " << attr.value << std::endl;
        else if (attr.name == "blockSize")
            std::cout << "block size: " << attr.value << std::endl;
        else if (attr.name == "keyBits")
            std::cout << "key bits: " << attr.value << std::endl;
        else if (attr.name == "hashSize")
            std::cout << "hash size: " << attr.value << std::endl;
        else if (attr.name == "cipherAlgorithm")
            std::cout << "cipher algorithm: " << attr.value << std::endl;
        else if (attr.name == "cipherChaining")
            std::cout << "cipher chaining: " << attr.value << std::endl;
        else if (attr.name == "hashAlgorithm")
            std::cout << "hash algorithm: " << attr.value << std::endl;
        else if (attr.name == "saltValue")
            print_base64("salt value", attr.value);
        else if (attr.name == "encryptedVerifierHashInput")
            print_base64("encrypted verifier hash input", attr.value);
        else if (attr.name == "encryptedVerifierHashValue")
            print_base64("encrypted verifier hash value", attr.value);
        else if (attr.name == "encryptedKeyValue")
            print_base64("encrypted key value", attr.value);

    }
};

class sax_handler
{
    std::vector<sax_ns_parser_attribute> m_attrs;

public:
    sax_handler(xmlns_context& /*ns_cxt*/) {}
    void doctype(const sax::doctype_declaration&) {}
    void start_declaration(std::string_view) {}
    void end_declaration(std::string_view) {}
    void attribute(std::string_view, std::string_view) {}

    void attribute(const sax_ns_parser_attribute& attr)
    {
        m_attrs.push_back(attr);
    }

    void characters(std::string_view, bool) {}

    void start_element(const sax_ns_parser_element& elem)
    {
        if (elem.ns == NS_mso_encryption)
        {
            if (elem.name == "keyData")
            {
                std::cout << "--- key data" << std::endl;
                key_data_attr_handler func;
                for_each(m_attrs.begin(), m_attrs.end(), func);
            }
            else if (elem.name == "dataIntegrity")
            {
                std::cout << "--- data integrity" << std::endl;
                data_integrity_attr_handler func;
                for_each(m_attrs.begin(), m_attrs.end(), func);
            }
        }
        else if (elem.ns == NS_mso_password)
        {
            if (elem.name == "encryptedKey")
            {
                std::cout << "--- encrypted key" << std::endl;
                password_encrypted_key_attr_handler func;
                for_each(m_attrs.begin(), m_attrs.end(), func);
            }
        }

        m_attrs.clear();
    }

    void end_element(const sax_ns_parser_element&) {}
};

}

struct encryption_info_reader_impl
{
    orcus::xmlns_repository m_ns_repo;

    encryption_info_reader_impl()
    {
        m_ns_repo.add_predefined_values(NS_mso_all);
    }
};

encryption_info_reader::encryption_info_reader() :
    mp_impl(new encryption_info_reader_impl) {}

encryption_info_reader::~encryption_info_reader()
{
    delete mp_impl;
}

void encryption_info_reader::read(const char* p, size_t n)
{
#if ORCUS_DEBUG_MSO_ENCRYPTION_INFO
    std::cout << "encryption_info_reader::read: stream size=" << n << std::endl;
#endif
    orcus::xmlns_context cxt = mp_impl->m_ns_repo.create_context();
    sax_handler hdl(cxt);
    orcus::sax_ns_parser<sax_handler> parser({p, n}, cxt, hdl);
    parser.parse();
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
