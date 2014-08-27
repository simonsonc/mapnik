/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_EXPRESSION_NODE_HPP
#define MAPNIK_EXPRESSION_NODE_HPP

// mapnik
#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/expression_node_types.hpp>

// boost
#if defined(BOOST_REGEX_HAS_ICU)
#include <boost/regex/icu.hpp>
#else
#include <boost/regex.hpp>
#endif

namespace mapnik
{

using value_type = mapnik::value;

template <typename Tag> struct make_op;
template <> struct make_op<tags::negate> { using type = std::negate<value_type>;};
template <> struct make_op<tags::plus> { using type = std::plus<value_type>;};
template <> struct make_op<tags::minus> { using type = std::minus<value_type>;};
template <> struct make_op<tags::mult> { using type = std::multiplies<value_type>;};
template <> struct make_op<tags::div> { using type = std::divides<value_type>;};
template <> struct make_op<tags::mod> { using type =  std::modulus<value_type>;};
template <> struct make_op<tags::less> { using type = std::less<value_type>;};
template <> struct make_op<tags::less_equal> { using type = std::less_equal<value_type>;};
template <> struct make_op<tags::greater> { using type = std::greater<value_type>;};
template <> struct make_op<tags::greater_equal> { using type = std::greater_equal<value_type>;};
template <> struct make_op<tags::equal_to> { using type = std::equal_to<value_type>;};
template <> struct make_op<tags::not_equal_to> { using type = std::not_equal_to<value_type>;};
template <> struct make_op<tags::logical_not> { using type = std::logical_not<value_type>;};
template <> struct make_op<tags::logical_and> { using type = std::logical_and<value_type>;};
template <> struct make_op<tags::logical_or> { using type =  std::logical_or<value_type>;};

template <typename Tag>
struct unary_node
{
    unary_node (expr_node const& a)
        : expr(a) {}

    static const char* type()
    {
        return Tag::str();
    }

    expr_node expr;
};

template <typename Tag>
struct binary_node
{
    binary_node(expr_node const& a, expr_node const& b)
        : left(a),
          right(b) {}

    static const char* type()
    {
        return Tag::str();
    }
    expr_node left,right;
};

#if defined(BOOST_REGEX_HAS_ICU)

struct regex_match_node
{
    regex_match_node (expr_node const& a, mapnik::value_unicode_string const& ustr);
    expr_node expr;
    boost::u32regex pattern;
};


struct regex_replace_node
{
    regex_replace_node (expr_node const& a, mapnik::value_unicode_string const& ustr, mapnik::value_unicode_string const& f);
    expr_node expr;
    boost::u32regex pattern;
    mapnik::value_unicode_string format;
};

#else

struct regex_match_node
{
    regex_match_node (expr_node const& a, std::string const& str);
    expr_node expr;
    boost::regex pattern;
};


struct regex_replace_node
{
    regex_replace_node (expr_node const& a, std::string const& str, std::string const& f);
    expr_node expr;
    boost::regex pattern;
    std::string format;
};
#endif

using unary_function_impl = std::function<value_type(value_type const&)>;
using binary_function_impl = std::function<value_type(value_type const&, value_type const&)>;

struct unary_function_call
{
    using argument_type = expr_node;
    unary_function_call() = default;
    unary_function_call(unary_function_impl fun, argument_type const& arg)
        : fun(fun), arg(arg) {}

    unary_function_impl fun;
    argument_type arg;
};

struct binary_function_call
{
    using argument_type = expr_node;
    binary_function_call() = default;
    binary_function_call(binary_function_impl fun, argument_type const& arg1, argument_type const& arg2)
        : fun(fun), arg1(arg1), arg2(arg2) {}
    binary_function_impl fun;
    argument_type arg1;
    argument_type arg2;
};


inline expr_node & operator- (expr_node& expr)
{
    return expr = unary_node<tags::negate>(expr);
}

inline expr_node & operator += ( expr_node &left, expr_node const& right)
{
    return left =  binary_node<tags::plus>(left,right);
}

inline expr_node & operator -= ( expr_node &left, expr_node const& right)
{
    return left =  binary_node<tags::minus>(left,right);
}

inline expr_node & operator *= ( expr_node &left , expr_node const& right)
{
    return left =  binary_node<tags::mult>(left,right);
}

inline expr_node & operator /= ( expr_node &left , expr_node const& right)
{
    return left =  binary_node<tags::div>(left,right);
}

inline expr_node & operator %= ( expr_node &left , expr_node const& right)
{
    return left = binary_node<tags::mod>(left,right);
}

inline expr_node & operator < ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::less>(left,right);
}

inline expr_node & operator <= ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::less_equal>(left,right);
}

inline expr_node & operator > ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::greater>(left,right);
}

inline expr_node & operator >= ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::greater_equal>(left,right);
}

inline expr_node & operator == ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::equal_to>(left,right);
}

inline expr_node & operator != ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::not_equal_to>(left,right);
}

inline expr_node & operator ! (expr_node & expr)
{
    return expr = unary_node<tags::logical_not>(expr);
}

inline expr_node & operator && ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::logical_and>(left,right);
}

inline expr_node & operator || ( expr_node &left, expr_node const& right)
{
    return left = binary_node<tags::logical_or>(left,right);
}

}


#endif //MAPNIK_EXPRESSION_NODE_HPP
