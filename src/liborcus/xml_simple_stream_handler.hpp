/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_XML_SIMPLE_HANDLER_HPP__
#define __ORCUS_XML_SIMPLE_HANDLER_HPP__

#include "xml_stream_handler.hpp"

namespace orcus {

class xml_context_base;

/**
 * Simple stream handler that only uses a single context instance.  Note that
 * when using this handler, element validation rules are ignored.
 *
 * TODO: Phase out the use of this class.
 */
class xml_simple_stream_handler : public xml_stream_handler
{
public:
    xml_simple_stream_handler(
        session_context& session_cxt, const tokens& t, std::unique_ptr<xml_context_base> context);
    virtual ~xml_simple_stream_handler() override;

    xml_context_base& get_context();

    virtual void start_document() override;
    virtual void end_document() override;

    virtual void start_element(const xml_token_element_t& elem) override;
    virtual void end_element(const xml_token_element_t& elem) override;
    virtual void characters(std::string_view str, bool transient) override;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
