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

// mapnik
#include <mapnik/group/group_symbolizer_helper.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/vertex_cache.hpp>
#include <mapnik/text/tolerance_iterator.hpp>

//agg
#include "agg_conv_clip_polyline.h"

namespace mapnik {

group_symbolizer_helper::group_symbolizer_helper(
        const group_symbolizer &sym, const feature_impl &feature,
        attributes const& vars,
        const proj_transform &prj_trans,
        unsigned width, unsigned height, double scale_factor,
        const CoordTransform &t, DetectorType &detector,
        const box2d<double> &query_extent)
    : base_symbolizer_helper(sym, feature, vars, prj_trans, width, height, scale_factor, t, query_extent),
      detector_(detector)
{}

pixel_position_list const& group_symbolizer_helper::get()
{
    results_.clear();

    if (point_placement_)
    {
        for (pixel_position const& point : points_)
        {
            check_point_placement(point);
        }
    }
    else
    {
        for (auto const& geom : geometries_to_process_)
        {
            if (clipped_)
            {
                using clipped_geometry_type = agg::conv_clip_polyline<geometry_type>;
                using path_type = coord_transform<CoordTransform,clipped_geometry_type>;

                clipped_geometry_type clipped(*geom);
                clipped.clip_box(query_extent_.minx(), query_extent_.miny(),
                                 query_extent_.maxx(), query_extent_.maxy());
                path_type path(t_, clipped, prj_trans_);
                find_line_placements(path);
            }
            else
            {
                using path_type = coord_transform<CoordTransform,geometry_type>;
                path_type path(t_, *geom, prj_trans_);
                find_line_placements(path);
            }
        }
    }

    return results_;
}

template <typename T>
bool group_symbolizer_helper::find_line_placements(T & path)
{
    if (box_elements_.empty()) return true;

    vertex_cache pp(path);

    bool success = false;
    while (pp.next_subpath())
    {
        if (pp.length() <= 0.001)
        {
            success = check_point_placement(pp.current_position()) || success;
            continue;
        }

        double spacing = get_spacing(pp.length());

        pp.forward(spacing/2.0);
        do
        {
            tolerance_iterator tolerance_offset(placement_->properties.label_position_tolerance * scale_factor_, spacing); //TODO: Handle halign
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(pp);
                if (pp.move(tolerance_offset.get()) && check_point_placement(pp.current_position()))
                {
                    success = true;
                    break;
                }
            }
        } while (pp.forward(spacing));
    }
    return success;
}

bool group_symbolizer_helper::check_point_placement(pixel_position const& pos)
{
    if (box_elements_.empty()) return false;

    // offset boxes and check collision
    std::list< box2d<double> > real_boxes;
    for (auto const& box_elem : box_elements_)
    {
        box2d<double> real_box = box2d<double>(box_elem.box_);
        real_box.move(pos.x, pos.y);
        if (collision(real_box, box_elem.repeat_key_))
        {
            return false;
        }
        real_boxes.push_back(real_box);
    }

    // add boxes to collision detector
    std::list<box_element>::iterator elem_itr = box_elements_.begin();
    std::list< box2d<double> >::iterator real_itr = real_boxes.begin();
    while (elem_itr != box_elements_.end() && real_itr != real_boxes.end())
    {
        detector_.insert(*real_itr, elem_itr->repeat_key_);
        elem_itr++;
        real_itr++;
    }

    results_.push_back(pos);

    return true;
}

bool group_symbolizer_helper::collision(const box2d<double> &box, const value_unicode_string &repeat_key) const
{
    if (!detector_.extent().intersects(box)
            ||
        (placement_->properties.avoid_edges && !query_extent_.contains(box))
            ||
        (placement_->properties.minimum_padding > 0 &&
         !query_extent_.contains(box + (scale_factor_ * placement_->properties.minimum_padding)))
            ||
        (!placement_->properties.allow_overlap &&
            ((repeat_key.length() == 0 && !detector_.has_placement(box, placement_->properties.minimum_distance * scale_factor_))
                ||
             (repeat_key.length() > 0  && !detector_.has_placement(box, placement_->properties.minimum_distance * scale_factor_,
                                                                   repeat_key, placement_->properties.repeat_distance * scale_factor_))))
        )
    {
        return true;
    }
    return false;
}

double group_symbolizer_helper::get_spacing(double path_length) const
{
    int num_labels = 1;
    if (placement_->properties.label_spacing > 0)
    {
        num_labels = static_cast<int>(floor(
            path_length / (placement_->properties.label_spacing * scale_factor_)));
    }
    if (num_labels <= 0)
    {
        num_labels = 1;
    }
    return path_length / num_labels;
}

} //namespace
