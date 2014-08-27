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

#ifndef MAPNIK_TOPOJSON_UTILS_HPP
#define MAPNIK_TOPOJSON_UTILS_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik { namespace topojson {

struct bounding_box_visitor : public mapnik::util::static_visitor<box2d<double> >
{
    bounding_box_visitor(topology const& topo)
        : topo_(topo) {}

    box2d<double> operator() (mapnik::topojson::point const& pt) const
    {
        double x = pt.coord.x;
        double y = pt.coord.y;
        if (topo_.tr)
        {
            x =  x * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
            y =  y * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
        }
        return box2d<double>(x, y, x, y);
    }

    box2d<double> operator() (mapnik::topojson::multi_point const& multi_pt) const
    {
        box2d<double> bbox;
        bool first = true;
        for (auto const& pt : multi_pt.points)
        {
            double x = pt.x;
            double y = pt.y;
            if (topo_.tr)
            {
                x =  x * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                y =  y * (*topo_.tr).scale_y + (*topo_.tr).translate_y; // TODO : delta encoded ?
            }
            if (first)
            {
                first = false;
                bbox.init(x,y,x,y);
            }
            else
            {
                bbox.expand_to_include(x,y);
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::linestring const& line) const
    {
        box2d<double> bbox;
        bool first = true;
        double px = 0, py = 0;
        index_type arc_index = line.ring;
        for (auto pt : topo_.arcs[arc_index].coordinates)
        {
            double x = pt.x;
            double y = pt.y;
            if (topo_.tr)
            {
                x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
            }
            if (first)
            {
                first = false;
                bbox.init(x, y, x, y);
            }
            else
            {
                bbox.expand_to_include(x, y);
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::multi_linestring const& multi_line) const
    {
        box2d<double> bbox;
        bool first = true;
        for (auto index : multi_line.rings)
        {
            double px = 0, py = 0;
            index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
            for (auto pt : topo_.arcs[arc_index].coordinates)
            {
                double x = pt.x;
                double y = pt.y;
                if (topo_.tr)
                {
                    x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                    y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                }
                if (first)
                {
                    first = false;
                    bbox.init(x, y, x, y);
                }
                else
                {
                    bbox.expand_to_include(x, y);
                }
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::polygon const& poly) const
    {
        box2d<double> bbox;
        bool first = true;
        for (auto const& ring : poly.rings)
        {
            for (auto index : ring)
            {
                double px = 0, py = 0;
                index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                for (auto const& pt : topo_.arcs[arc_index].coordinates)
                {
                    double x = pt.x;
                    double y = pt.y;

                    if (topo_.tr)
                    {
                        x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                        y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                    }

                    if (first)
                    {
                        first = false;
                        bbox.init( x, y, x, y);
                    }
                    else
                    {
                        bbox.expand_to_include(x, y);
                    }
                }
            }
        }
        return bbox;
    }

    box2d<double> operator() (mapnik::topojson::multi_polygon const& multi_poly) const
    {
        box2d<double> bbox;
        bool first = true;
        for (auto const& poly : multi_poly.polygons)
        {
            for (auto const& ring : poly)
            {
                for (auto index : ring)
                {
                    double px = 0, py = 0;
                    index_type arc_index = index < 0 ? std::abs(index) - 1 : index;
                    for (auto const& pt : topo_.arcs[arc_index].coordinates)
                    {
                        double x = pt.x;
                        double y = pt.y;

                        if (topo_.tr)
                        {
                            x =  (px += x) * (*topo_.tr).scale_x + (*topo_.tr).translate_x;
                            y =  (py += y) * (*topo_.tr).scale_y + (*topo_.tr).translate_y;
                        }

                        if (first)
                        {
                            first = false;
                            bbox.init( x, y, x, y);
                        }
                        else
                        {
                            bbox.expand_to_include(x, y);
                        }
                    }
                }
            }
        }
        return bbox;
    }

    template<typename T>
    box2d<double> operator() (T const& ) const
    {
        return box2d<double>();
    }

private:
    topology const& topo_;
};

}}

#endif //MAPNIK_TOPOJSON_UTILS_HPP
