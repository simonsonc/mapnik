/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
#define MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/path_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>
#include <mapnik/vertex.hpp>    // for CommandType::SEG_MOVETO

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc

//stl
#include <tuple>

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

namespace detail {

struct get_type
{
    using result_type = int;
    result_type operator() (geometry_type const& geom) const
    {
        return static_cast<int>(geom.type());
    }
};

struct get_first
{
    using result_type = geometry_type::value_type const;
    result_type operator() (geometry_type const& geom) const
    {
        geometry_type::value_type coord;
        std::get<0>(coord) = geom.vertex(0,&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};

struct multi_geometry_type
{
    using result_type = std::tuple<unsigned,bool> ;
    result_type operator() (geometry_container const& cont) const
    {
        unsigned type = 0u;
        bool collection = false;
        for (auto const& geom : cont)
        {
            if (type != 0u && geom.type() != type)
            {
                collection = true;
                break;
            }
            type = geom.type();
        }
        if (cont.size() > 1) type +=3;
        return std::tuple<unsigned,bool>(type, collection);
    }
};

struct not_empty
{
    using result_type = bool;
    result_type operator() (geometry_container const& cont) const
    {
        for (auto const& geom : cont)
        {
            if (geom.size() > 0) return true;
        }
        return false;
    }
};

template <typename T>
struct json_coordinate_policy : karma::real_policies<T>
{
    using base_type = boost::spirit::karma::real_policies<T>;
    static int floatfield(T n) { return base_type::fmtflags::fixed; }

    static unsigned precision(T n)
    {
        if (n == 0.0) return 0;
        using namespace boost::spirit;
        return static_cast<unsigned>(14 - boost::math::trunc(log10(traits::get_absolute_value(n))));
    }

    template <typename OutputIterator>
    static bool dot(OutputIterator& sink, T n, unsigned precision)
    {
        if (n == 0) return true;
        return base_type::dot(sink, n, precision);
    }

    template <typename OutputIterator>
    static bool fraction_part(OutputIterator& sink, T n
                              , unsigned adjprec, unsigned precision)
    {
        if (n == 0) return true;
        return base_type::fraction_part(sink, n, adjprec, precision);
    }
};

}

template <typename OutputIterator>
struct geometry_generator_grammar :
        karma::grammar<OutputIterator, geometry_type const& ()>
{
    geometry_generator_grammar();
    karma::rule<OutputIterator, geometry_type const& ()> coordinates;
    karma::rule<OutputIterator, geometry_type const& ()> point;
    karma::rule<OutputIterator, geometry_type const& ()> linestring;
    karma::rule<OutputIterator, geometry_type const& ()> polygon;
    karma::rule<OutputIterator, geometry_type const& ()> coords;
    karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, geometry_type::value_type ()> point_coord;
    karma::rule<OutputIterator, geometry_type::value_type (unsigned& )> polygon_coord;
    boost::phoenix::function<detail::get_type > _type;
    boost::phoenix::function<detail::get_first> _first;
    karma::real_generator<double, detail::json_coordinate_policy<double> > coord_type;
};


template <typename OutputIterator>
struct multi_geometry_generator_grammar :
        karma::grammar<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                       geometry_container const& ()>
{
    multi_geometry_generator_grammar();
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> start;
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> geometry_collection;
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> geometry;
    karma::rule<OutputIterator, karma::locals<unsigned>,
                geometry_type const&()> geometry2;
    karma::rule<OutputIterator, geometry_container const&()> coordinates;
    geometry_generator_grammar<OutputIterator>  path;
    boost::phoenix::function<detail::multi_geometry_type> multi_type_;
    boost::phoenix::function<detail::get_type > type_;
    boost::phoenix::function<detail::not_empty> not_empty_;
    karma::symbols<unsigned, char const*> geometry_types;
};

}}

#endif // MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
