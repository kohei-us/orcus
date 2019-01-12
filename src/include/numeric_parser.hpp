/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_DETAIL_NUMERIC_PARSER_HPP
#define INCLUDED_ORCUS_DETAIL_NUMERIC_PARSER_HPP

#include <limits>
#include <cmath>

namespace orcus { namespace detail {

struct generic_parser_trait
{
};

struct json_parser_trait
{
};

template<typename _Trait>
class numeric_parser
{
    using trait_type = _Trait;

    const char* mp_char;
    const char* mp_end;

    bool check_sign()
    {
        bool negative_sign = false;

        // Check for presence of a sign.
        if (mp_char != mp_end)
        {
            switch (*mp_char)
            {
                case '+':
                    ++mp_char;
                    break;
                case '-':
                    negative_sign = true;
                    ++mp_char;
                    break;
                default:
                    ;
            }
        }

        return negative_sign;
    }

    /**
     * Parse the exponent part of a numeric string.
     *
     * @return extra divisor to multiply to the original divisor, or 0.0 if the
     *         parsing fails.
     */
    double parse_exponent()
    {
        const char* p0 = mp_char - 1; // original position to restore to in case of parsing failure. The e needs to be added back as well.
        double exponent = 0.0;
        bool valid = false;

        bool negative_sign = check_sign();

        for (; mp_char != mp_end; ++mp_char)
        {
            if (*mp_char < '0' || '9' < *mp_char)
            {
                // Non-digit encountered.
                break;
            }

            valid = true;
            exponent *= 10.0;
            exponent += *mp_char - '0';
        }

        if (!valid)
        {
            // Restore the original position on failed parsing.
            mp_char = p0;
            return 0.0;
        }

        if (!negative_sign)
            exponent = -exponent;

        return std::pow(10.0, exponent);
    }

public:
    numeric_parser(const char* p, const char* p_end) : mp_char(p), mp_end(p_end) {}

    /**
     * Start parsing the string.
     *
     * @return a finite value upon successful parsing, else NaN is returned.
     */
    double parse()
    {
        double ret = 0.0, divisor = 1.0;
        bool before_decimal_pt = true;
        bool has_digit = false;

        bool negative_sign = check_sign();

        for (; mp_char != mp_end; ++mp_char)
        {
            if (*mp_char == '.')
            {
                if (!before_decimal_pt)
                {
                    // Second '.' encountered. Terminate the parsing.
                    ret /= divisor;
                    return negative_sign ? -ret : ret;
                }

                before_decimal_pt = false;
                continue;
            }

            if (has_digit && (*mp_char == 'e' || *mp_char == 'E'))
            {
                ++mp_char;
                double extra_divisor = parse_exponent();
                if (extra_divisor)
                    divisor *= extra_divisor;
                break;
            }

            if (*mp_char < '0' || '9' < *mp_char)
            {
                if (!has_digit) // without a digit we have no numbers
                    return std::numeric_limits<double>::quiet_NaN();

                ret /= divisor;
                return negative_sign ? -ret : ret;
            }

            has_digit = true;
            ret *= 10.0;
            ret += *mp_char - '0';

            if (!before_decimal_pt)
                divisor *= 10.0;
        }
        if (!has_digit) // without a digit we have no numbers
            return std::numeric_limits<double>::quiet_NaN();

        ret /= divisor;
        return negative_sign ? -ret : ret;
    }

    const char* get_char_position() const
    {
        return mp_char;
    }
};

}} // namespace orcus::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
