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
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/debug.hpp>

//agg
#include "agg_conv_clip_polyline.h"

namespace mapnik {

template <typename FaceManagerT, typename DetectorT>
text_symbolizer_helper::text_symbolizer_helper(const text_symbolizer &sym, const feature_impl &feature, const proj_transform &prj_trans, unsigned width, unsigned height, double scale_factor, const CoordTransform &t, FaceManagerT &font_manager, DetectorT &detector, const box2d<double> &query_extent)
    : sym_(sym),
      feature_(feature),
      prj_trans_(prj_trans),
      t_(t),
      dims_(0, 0, width, height),
      query_extent_(query_extent),
      points_on_line_(false),
      placement_(sym_.get_placement_options()->get_placement_info(scale_factor)),
      finder_(feature, detector, dims_, placement_, font_manager, scale_factor)
{
    initialize_geometries();
    if (!geometries_to_process_.size()) return;
    finder_.next_position();
    initialize_points();
}

placements_list const& text_symbolizer_helper::get()
{
    if (point_placement_)
    {
        while (next_point_placement());
    }
    else
    {
        while (next_line_placement());
    }
    return finder_.placements();
}

bool text_symbolizer_helper::next_line_placement()
{
    while (!geometries_to_process_.empty())
    {
        if (geo_itr_ == geometries_to_process_.end())
        {
            //Just processed the last geometry. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            geo_itr_ = geometries_to_process_.begin();
            continue; //Reexecute size check
        }

        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
        typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

        clipped_geometry_type clipped(**geo_itr_);
        clipped.clip_box(query_extent_.minx(), query_extent_.miny(),
                         query_extent_.maxx(), query_extent_.maxy());
        path_type path(t_, clipped, prj_trans_);
        bool success = finder_.find_line_placements(path, points_on_line_);
        if (success)
        {
            //Found a placement
            geo_itr_ = geometries_to_process_.erase(geo_itr_);
            return true;
        }
        //No placement for this geometry. Keep it in geometries_to_process_ for next try.
        geo_itr_++;
    }
    return false;
}

bool text_symbolizer_helper::next_point_placement()
{
    while (!points_.empty())
    {
        if (point_itr_ == points_.end())
        {
            //Just processed the last point. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            point_itr_ = points_.begin();
            continue; //Reexecute size check
        }
        if (finder_.find_point_placement(*point_itr_))
        {
            //Found a placement
            point_itr_ = points_.erase(point_itr_);
            return true;
        }
        //No placement for this point. Keep it in points_ for next try.
        point_itr_++;
    }
    return false;
}

struct largest_bbox_first
{
    bool operator() (geometry_type const* g0, geometry_type const* g1) const
    {
        box2d<double> b0 = g0->envelope();
        box2d<double> b1 = g1->envelope();
        return b0.width()*b0.height() > b1.width()*b1.height();
    }

};

void text_symbolizer_helper::initialize_geometries()
{
    bool largest_box_only = false;
    std::size_t num_geom = feature_.num_geometries();
    for (std::size_t i=0; i<num_geom; ++i)
    {
        geometry_type const& geom = feature_.get_geometry(i);

        // don't bother with empty geometries
        if (geom.size() == 0) continue;
        mapnik::geometry_type::types type = geom.type();
        if (type == geometry_type::types::Polygon)
        {
            largest_box_only = sym_.largest_bbox_only();
            if (sym_.get_minimum_path_length() > 0)
            {
                box2d<double> gbox = t_.forward(geom.envelope(), prj_trans_);
                if (gbox.width() < sym_.get_minimum_path_length())
                {
                    continue;
                }
            }
        }
        // TODO - calculate length here as well
        geometries_to_process_.push_back(const_cast<geometry_type*>(&geom));
    }

    if (largest_box_only)
    {
        geometries_to_process_.sort(largest_bbox_first());
        geo_itr_ = geometries_to_process_.begin();
        geometries_to_process_.erase(++geo_itr_,geometries_to_process_.end());
    }
    geo_itr_ = geometries_to_process_.begin();
}

void text_symbolizer_helper::initialize_points()
{
    label_placement_enum how_placed = placement_->properties.label_placement;
    if (how_placed == LINE_PLACEMENT)
    {
        point_placement_ = false;
        return;
    }
    else
    {
        point_placement_ = true;
    }

    double label_x=0.0;
    double label_y=0.0;
    double z=0.0;

    std::list<geometry_type*>::const_iterator itr = geometries_to_process_.begin();
    std::list<geometry_type*>::const_iterator end = geometries_to_process_.end();
    for (; itr != end; itr++)
    {
        geometry_type const& geom = **itr;
        if (how_placed == VERTEX_PLACEMENT)
        {
            geom.rewind(0);
            for(unsigned i = 0; i < geom.size(); i++)
            {
                geom.vertex(&label_x, &label_y);
                prj_trans_.backward(label_x, label_y, z);
                t_.forward(&label_x, &label_y);
                points_.push_back(pixel_position(label_x, label_y));
            }
        }
        else
        {
            // https://github.com/mapnik/mapnik/issues/1423
            bool success = false;
            // https://github.com/mapnik/mapnik/issues/1350
            if (geom.type() == geometry_type::types::LineString)
            {
                success = label::middle_point(geom, label_x,label_y);
            }
            else if (how_placed == POINT_PLACEMENT)
            {
                success = label::centroid(geom, label_x, label_y);
            }
            else if (how_placed == INTERIOR_PLACEMENT)
            {
                success = label::interior_position(geom, label_x, label_y);
            }
            else
            {
                MAPNIK_LOG_ERROR(symbolizer_helpers) << "ERROR: Unknown placement type in initialize_points()";
            }
            if (success)
            {
                prj_trans_.backward(label_x, label_y, z);
                t_.forward(&label_x, &label_y);
                points_.push_back(pixel_position(label_x, label_y));
            }
        }
    }
    point_itr_ = points_.begin();
}

/*****************************************************************************/

template <typename FaceManagerT, typename DetectorT>
text_symbolizer_helper::text_symbolizer_helper(
        const shield_symbolizer &sym, const feature_impl &feature,
        const proj_transform &prj_trans,
        unsigned width, unsigned height, double scale_factor,
        const CoordTransform &t, FaceManagerT &font_manager,
        DetectorT &detector, const box2d<double> &query_extent)
    : sym_(sym),
      feature_(feature),
      prj_trans_(prj_trans),
      t_(t),
      dims_(0, 0, width, height),
      query_extent_(query_extent),
      points_on_line_(true),
      placement_(sym_.get_placement_options()->get_placement_info(scale_factor)),
      finder_(feature, detector, dims_, placement_, font_manager, scale_factor)
{
    initialize_geometries();
    if (!geometries_to_process_.size()) return;
    finder_.next_position();
    initialize_points();
    init_marker();
}


void text_symbolizer_helper::init_marker()
{
    shield_symbolizer const& sym = static_cast<shield_symbolizer const&>(sym_);
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature_);
    agg::trans_affine trans;
    evaluate_transform(trans, feature_, sym.get_image_transform());
    boost::optional<marker_ptr> opt_marker; //TODO: Why boost::optional?
    if (!filename.empty())
    {
        opt_marker = marker_cache::instance().find(filename, true);
    }
    marker_ptr m;
    if (opt_marker) m = *opt_marker;
    if (!m) return;
    double width = m->width();
    double height = m->height();
    double px0 = - 0.5 * width;
    double py0 = - 0.5 * height;
    double px1 = 0.5 * width;
    double py1 = 0.5 * height;
    double px2 = px1;
    double py2 = py0;
    double px3 = px0;
    double py3 = py1;
    trans.transform(&px0, &py0);
    trans.transform(&px1, &py1);
    trans.transform(&px2, &py2);
    trans.transform(&px3, &py3);
    box2d<double> bbox(px0, py0, px1, py1);
    bbox.expand_to_include(px2, py2);
    bbox.expand_to_include(px3, py3);
    finder_.set_marker(std::make_shared<marker_info>(m, trans), bbox, sym.get_unlock_image(), sym.get_shield_displacement());
}

template text_symbolizer_helper::text_symbolizer_helper(const text_symbolizer &sym, const feature_impl &feature,
const proj_transform &prj_trans,
unsigned width, unsigned height, double scale_factor,
const CoordTransform &t, face_manager<freetype_engine> &font_manager,
label_collision_detector4 &detector, const box2d<double> &query_extent);

template text_symbolizer_helper::text_symbolizer_helper(const shield_symbolizer &sym, const feature_impl &feature,
const proj_transform &prj_trans,
unsigned width, unsigned height, double scale_factor,
const CoordTransform &t, face_manager<freetype_engine> &font_manager,
label_collision_detector4 &detector, const box2d<double> &query_extent);
} //namespace
