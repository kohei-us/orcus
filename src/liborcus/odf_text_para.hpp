/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace orcus {

/**
 * Single segment of a paragraph.
 *
 * The text of a segment may consist of more than one text fragment.
 *
 * @note Add more optional format attributes as needed.
 */
struct odf_text_para_segment
{
    std::optional<std::size_t> font;
    std::vector<std::string_view> text_fragments;

    void reset();
};

using odf_text_paragraph = std::vector<odf_text_para_segment>;
using odf_text_paragraphs = std::vector<odf_text_paragraph>;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
