/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_DETAIL_NUMERIC_PARSER_HPP
#define INCLUDED_ORCUS_DETAIL_NUMERIC_PARSER_HPP

#include <mdds/global.hpp>
#include <limits>
#include <cmath>

namespace orcus { namespace detail {

struct generic_parser_trait {};

struct json_parser_trait {};

struct parser_state
{
    /** number of digits before the decimal point. */
    int int_digit_count = 0;
    /** number of digits after the decimal point. */
    int frac_digit_count = 0;
    /** first digit before the decimal point. */
    char first_int_digit = 0;
    double parsed_value = 0.0;
    double divisor = 1.0;
    bool has_digit = false;
    bool has_decimal = false;
    bool negative_sign = false;
};

template<typename _Trait>
double make_final_value(const parser_state& state);

template<>
inline double make_final_value<generic_parser_trait>(const parser_state& state)
{
    return state.negative_sign ? -state.parsed_value : state.parsed_value;
}

template<>
inline double make_final_value<json_parser_trait>(const parser_state& state)
{
    if (state.int_digit_count > 1 && state.first_int_digit == 0)
        // leading zeros not allowed.
        return std::numeric_limits<double>::quiet_NaN();

    if (state.has_decimal && (state.frac_digit_count == 0 || state.int_digit_count == 0))
        // at least one digit is required both before and after the decimal point.
        return std::numeric_limits<double>::quiet_NaN();

    return state.negative_sign ? -state.parsed_value : state.parsed_value;
}

template<typename _Trait>
class numeric_parser
{
    using trait_type = _Trait;

    const char* mp_char;
    const char* mp_end;

    parser_state m_state;

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
    numeric_parser(const char* p, const char* p_end) :
        mp_char(p),
        mp_end(p_end) {}

    /**
     * Start parsing the string.
     *
     * @return a finite value upon successful parsing, else NaN is returned.
     */
    double parse()
    {
        m_state.negative_sign = check_sign();

        for (; mp_char != mp_end; ++mp_char)
        {
            if (*mp_char == '.')
            {
                if (m_state.has_decimal)
                {
                    // Second '.' encountered. Terminate the parsing.
                    m_state.parsed_value /= m_state.divisor;
                    return make_final_value<trait_type>(m_state);
                }

                m_state.has_decimal = true;
                continue;
            }

            if (m_state.has_digit && (*mp_char == 'e' || *mp_char == 'E'))
            {
                ++mp_char;
                double extra_divisor = parse_exponent();
                if (extra_divisor)
                    m_state.divisor *= extra_divisor;
                break;
            }

            if (*mp_char < '0' || '9' < *mp_char)
            {
                if (!m_state.has_digit) // without a digit we have no numbers
                    return std::numeric_limits<double>::quiet_NaN();

                m_state.parsed_value /= m_state.divisor;
                return make_final_value<trait_type>(m_state);
            }

            m_state.has_digit = true;
            char digit = *mp_char - '0';

            if (m_state.has_decimal)
                ++m_state.frac_digit_count;
            else
            {
                if (!m_state.int_digit_count)
                    m_state.first_int_digit = digit;

                ++m_state.int_digit_count;
            }

            m_state.parsed_value *= 10.0;
            m_state.parsed_value += digit;

            if (m_state.has_decimal)
                m_state.divisor *= 10.0;
        }
        if (!m_state.has_digit) // without a digit we have no numbers
            return std::numeric_limits<double>::quiet_NaN();

        m_state.parsed_value /= m_state.divisor;
        return make_final_value<trait_type>(m_state);
    }

    const char* get_char_position() const
    {
        return mp_char;
    }
};

}} // namespace orcus::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
