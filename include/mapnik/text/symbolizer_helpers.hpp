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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

//mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/vertex_converters.hpp>

namespace mapnik {

template <typename T>
struct placement_finder_adapter
{
    using placement_finder_type = T;
    placement_finder_adapter(T & finder, bool points_on_line)
        : finder_(finder),
          points_on_line_(points_on_line) {}

    template <typename PathT>
    void add_path(PathT & path)
    {
        status_ = finder_.find_line_placements(path, points_on_line_);
    }

    bool status() const { return status_;}
    // Place text at points on a line instead of following the line (used for ShieldSymbolizer)
    placement_finder_type & finder_;
    bool points_on_line_;
    mutable bool status_ = false;

};

using conv_types = boost::mpl::vector<clip_line_tag , transform_tag, affine_transform_tag, simplify_tag, smooth_tag>;
using vertex_converter_type = vertex_converter<box2d<double>, placement_finder_adapter<placement_finder> , symbolizer_base,
                                               CoordTransform, proj_transform, agg::trans_affine,
                                               conv_types, feature_impl>;

class base_symbolizer_helper
{
public:
    base_symbolizer_helper(symbolizer_base const& sym,
                           feature_impl const& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const& t,
                           box2d<double> const& query_extent);

protected:
    void initialize_geometries();
    void initialize_points();

    //Input
    symbolizer_base const& sym_;
    feature_impl const& feature_;
    attributes const& vars_;
    proj_transform const& prj_trans_;
    CoordTransform const& t_;
    box2d<double> dims_;
    box2d<double> const& query_extent_;
    float scale_factor_;
    bool clipped_;

    //Processing
    // Using list instead of vector, because we delete random elements and need iterators to stay valid.
    // Remaining geometries to be processed.
    std::list<geometry_type*> geometries_to_process_;
    // Remaining points to be processed.
    std::list<pixel_position> points_;
    // Geometry currently being processed.
    std::list<geometry_type*>::iterator geo_itr_;
    // Point currently being processed.
    std::list<pixel_position>::iterator point_itr_;
    // Use point placement. Otherwise line placement is used.
    bool point_placement_;

    text_placement_info_ptr placement_;
};

// Helper object that does all the TextSymbolizer placement finding
// work except actually rendering the object.

class text_symbolizer_helper : public base_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    text_symbolizer_helper(text_symbolizer const& sym,
                           feature_impl const& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const& t,
                           FaceManagerT & font_manager,
                           DetectorT & detector,
                           box2d<double> const& query_extent,
                           agg::trans_affine const&);

    template <typename FaceManagerT, typename DetectorT>
    text_symbolizer_helper(shield_symbolizer const& sym,
                           feature_impl const& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const& t,
                           FaceManagerT & font_manager,
                           DetectorT & detector,
                           box2d<double> const& query_extent,
                           agg::trans_affine const&);

    // Return all placements.
    placements_list const& get();
protected:
    bool next_point_placement();
    bool next_line_placement(bool clipped);

    placement_finder finder_;

    placement_finder_adapter<placement_finder> adapter_;
    vertex_converter_type converter_;
    //ShieldSymbolizer only
    void init_marker();
};

} //namespace

#endif // SYMBOLIZER_HELPERS_HPP
