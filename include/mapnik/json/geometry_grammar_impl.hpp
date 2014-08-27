/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/json/positions_grammar_impl.hpp>

// boost
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <iostream>                     // for clog, endl, etc
#include <string>                       // for string

namespace mapnik { namespace json {

template <typename Iterator>
geometry_grammar<Iterator>::geometry_grammar()
    : geometry_grammar::base_type(start,"geometry")
{

    qi::lit_type lit;
    qi::int_type int_;
    qi::double_type double_;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_r1_type _r1;
    qi::eps_type eps;
    using qi::fail;
    using qi::on_error;

    start = geometry(_r1) | geometry_collection(_r1);

    geometry = (lit('{')[_a = 0 ]
                >> (-lit(',') >> lit("\"type\"") >> lit(':') >> geometry_type_dispatch[_a = _1]
                    ^
                    (-lit(',') >> lit("\"coordinates\"") >> lit(':') >> coordinates[_b = _1]))[create_geometry(_r1,_a,_b)]
                >> lit('}'))
        | lit("null")
        ;

    geometry_collection = (lit('{')
                           >> (-lit(',') >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"")
                               ^
                               -lit(',') >> lit("\"geometries\"") >> lit(':')
                               >> lit('[') >> geometry(_r1) % lit(',') >> lit(']'))
                           >> lit('}'))
        | lit("null")
        ;

    geometry_type_dispatch.add
        ("\"Point\"",1)
        ("\"LineString\"",2)
        ("\"Polygon\"",3)
        ("\"MultiPoint\"",4)
        ("\"MultiLineString\"",5)
        ("\"MultiPolygon\"",6)
        ;

    // give some rules names
    geometry.name("Geometry");
    geometry_collection.name("GeometryCollection");
    geometry_type_dispatch.name("Geometry dispatch");
    coordinates.name("Coordinates");
    // error handler
    on_error<fail>
        (
            start
            , std::clog
            << boost::phoenix::val("Error! Expecting ")
            << _4  // what failed?
            << boost::phoenix::val(" here: \"")
            << where_message_(_3, _2, 16) // max 16 chars
            << boost::phoenix::val("\"")
            << std::endl
            );
}

}}
