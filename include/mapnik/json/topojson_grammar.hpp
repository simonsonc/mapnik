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

#ifndef MAPNIK_TOPOJSON_GRAMMAR_HPP
#define MAPNIK_TOPOJSON_GRAMMAR_HPP

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/util/variant.hpp>
// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

// stl
#include <string>

namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
namespace standard_wide = boost::spirit::standard_wide;
using standard_wide::space_type;

struct where_message
{
    using result_type = std::string;

    template <typename Iterator>
    std::string operator() (Iterator first, Iterator last, std::size_t size) const
    {
        std::string str(first, last);
        if (str.length() > size)
            return str.substr(0, size) + "..." ;
        return str;
    }
};

template <typename Iterator>
struct topojson_grammar : qi::grammar<Iterator, space_type, topology()>

{
    topojson_grammar();
private:
    // generic JSON support
    qi::rule<Iterator,space_type> value;
    qi::symbols<char const, char const> unesc_char;
    qi::uint_parser< unsigned, 16, 4, 4 > hex4 ;
    qi::int_parser<mapnik::value_integer,10,1,-1> int__;
    qi::rule<Iterator,std::string(), space_type> string_;
    qi::rule<Iterator,space_type> key_value;
    qi::rule<Iterator,space_type, util::variant<value_null,bool,
                                                value_integer,value_double,
                                                std::string>()> number;
    qi::rule<Iterator,space_type> object;
    qi::rule<Iterator,space_type> array;
    qi::rule<Iterator,space_type> pairs;
    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;

    // topoJSON
    qi::rule<Iterator, space_type, mapnik::topojson::topology()> topology;
    qi::rule<Iterator, space_type, std::vector<mapnik::topojson::geometry>()> objects;
    qi::rule<Iterator, space_type, std::vector<mapnik::topojson::arc>()> arcs;
    qi::rule<Iterator, space_type, mapnik::topojson::arc()> arc;
    qi::rule<Iterator, space_type, mapnik::topojson::coordinate()> coordinate;
    qi::rule<Iterator, space_type, mapnik::topojson::transform()> transform;
    qi::rule<Iterator, space_type, mapnik::topojson::bounding_box()> bbox;
    qi::rule<Iterator, space_type, mapnik::topojson::geometry() > geometry;
    qi::rule<Iterator, space_type, mapnik::topojson::point()> point;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_point()> multi_point;
    qi::rule<Iterator, space_type, mapnik::topojson::linestring()> linestring;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_linestring()> multi_linestring;
    qi::rule<Iterator, space_type, mapnik::topojson::polygon()> polygon;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_polygon()> multi_polygon;
    qi::rule<Iterator, space_type, void(std::vector<mapnik::topojson::geometry>&)> geometry_collection;

    qi::rule<Iterator, space_type, std::vector<index_type>()> ring;

    // properties
    qi::rule<Iterator, space_type, mapnik::topojson::properties()> properties;
    qi::rule<Iterator, space_type, mapnik::topojson::properties()> attributes;
    qi::rule<Iterator, space_type, mapnik::topojson::value()> attribute_value;
    // id
    qi::rule<Iterator,space_type> id;

    // error
    boost::phoenix::function<where_message> where_message_;
};

}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_HPP
