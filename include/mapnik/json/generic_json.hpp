/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_GENERIC_JSON_HPP
#define MAPNIK_GENERIC_JSON_HPP

#include <mapnik/util/variant.hpp>
#include <boost/spirit/include/qi.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

template <typename Iterator>
struct generic_json
{
    qi::rule<Iterator,space_type> value;
    qi::symbols<char const, char const> unesc_char;
    qi::uint_parser< unsigned, 16, 4, 4 > hex4 ;
    qi::int_parser<mapnik::value_integer,10,1,-1> int__;
    qi::rule<Iterator,std::string(), space_type> string_;
    qi::rule<Iterator,space_type> key_value;
    qi::rule<Iterator,mapnik::util::variant<value_null,bool,
                                            value_integer,value_double,
                                            std::string>(),space_type> number;
    qi::rule<Iterator,space_type> object;
    qi::rule<Iterator,space_type> array;
    qi::rule<Iterator,space_type> pairs;
    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
};


}}

#endif // MAPNIK_GENERIC_JSON_HPP
