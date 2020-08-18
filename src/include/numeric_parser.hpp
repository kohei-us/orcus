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
    static constexpr bool allow_leading_zeros = true;
};

struct json_parser_trait
{
    static constexpr bool allow_leading_zeros = false;
};

template<typename _Trait>
class numeric_parser
{
    using trait_type = _Trait;

    const char* mp_char;
    const char* mp_end;
    int m_digit_count;
    char m_first_digit;
    double m_parsed_value;
    double m_divisor;
    bool m_negative_sign;

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

    double make_final_value() const
    {
        if (!trait_type::allow_leading_zeros)
        {
            if (m_digit_count > 1 && m_first_digit == 0)
                return std::numeric_limits<double>::quiet_NaN();
        }

        return m_negative_sign ? -m_parsed_value : m_parsed_value;
    }

public:
    numeric_parser(const char* p, const char* p_end) :
        mp_char(p),
        mp_end(p_end),
        m_digit_count(0),
        m_first_digit(-1),
        m_parsed_value(0.0),
        m_divisor(1.0),
        m_negative_sign(false) {}

    /**
     * Start parsing the string.
     *
     * @return a finite value upon successful parsing, else NaN is returned.
     */
    double parse()
    {
        bool before_decimal_pt = true;
        m_negative_sign = check_sign();

        for (; mp_char != mp_end; ++mp_char)
        {
            if (*mp_char == '.')
            {
                if (!before_decimal_pt)
                {
                    // Second '.' encountered. Terminate the parsing.
                    m_parsed_value /= m_divisor;
                    return make_final_value();
                }

                before_decimal_pt = false;
                continue;
            }

            if (m_digit_count && (*mp_char == 'e' || *mp_char == 'E'))
            {
                ++mp_char;
                double extra_divisor = parse_exponent();
                if (extra_divisor)
                    m_divisor *= extra_divisor;
                break;
            }

            if (*mp_char < '0' || '9' < *mp_char)
            {
                if (!m_digit_count) // without a digit we have no numbers
                    return std::numeric_limits<double>::quiet_NaN();

                m_parsed_value /= m_divisor;
                return make_final_value();
            }

            char digit = *mp_char - '0';
            if (!m_digit_count)
                m_first_digit = digit;

            ++m_digit_count;
            m_parsed_value *= 10.0;
            m_parsed_value += digit;

            if (!before_decimal_pt)
                m_divisor *= 10.0;
        }
        if (!m_digit_count) // without a digit we have no numbers
            return std::numeric_limits<double>::quiet_NaN();

        m_parsed_value /= m_divisor;
        return make_final_value();
    }

    const char* get_char_position() const
    {
        return mp_char;
    }
};

}} // namespace orcus::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
