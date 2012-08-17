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
#ifndef MAPNIK_PLACEMENT_FINDER_NG_HPP
#define MAPNIK_PLACEMENT_FINDER_NG_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text_placements/base.hpp>
#include <mapnik/text/placements_list.hpp>
#include <mapnik/text/rotation.hpp>

//stl

//boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik
{

class label_collision_detector4;
typedef label_collision_detector4 DetectorType;

class feature_impl;
typedef feature_impl Feature;
class vertex_cache;

class placement_finder_ng : boost::noncopyable
{
public:
    placement_finder_ng(Feature const& feature,
                        DetectorType & detector,
                        box2d<double> const& extent,
                        text_placement_info_ptr placement_info,
                        face_manager_freetype & font_manager,
                        double scale_factor);

    /** Try to place a single label at the given point. */
    bool find_point_placement(pixel_position pos);
    /** Iterate over the given path, placing line-following labels with respect to label_spacing. */
    template <typename T>
    bool find_line_placements(T & path);
    /** Iterate over the given path, placing point labels with respect to label_spacing. */
    template <typename T>
    bool find_point_on_line_placements(T & path);
    /** Try next position alternative from placement_info. */
    bool next_position();

    placements_list const& placements() const;

    void set_marker(marker_info_ptr m, box2d<double> box, bool marker_unlocked, pixel_position const& marker_displacement);
private:
    void init_alignment();
    pixel_position alignment_offset() const;
    double jalign_offset(double line_width) const;

    bool single_line_placement(vertex_cache &pp, text_upright_e orientation);
    /** Moves dx pixels but makes sure not to fall of the end. */
    void path_move_dx(vertex_cache &pp);
    /** Normalize angle in range [-pi, +pi]. */
    static double normalize_angle(double angle);
    /** Adjusts user defined spacing to place an integer number of labels. */
    double get_spacing(double path_length, double layout_width) const;
    /** Checks for collision. */
    bool collision(box2d<double> const& box) const;
    /** Adds marker to glyph_positions and to collision detector. Returns false if there is a collision. */
    bool add_marker(glyph_positions_ptr glyphs, pixel_position const& pos) const;
    box2d<double> get_bbox(glyph_info const& glyph, pixel_position const& pos, rotation const& rot);
    Feature const& feature_;
    DetectorType &detector_;
    box2d<double> const& extent_;
    // Precalculated values for maximum performance
    rotation orientation_;
    text_layout layout_;
    text_placement_info_ptr info_;
    bool valid_;

    vertical_alignment_e valign_;
    horizontal_alignment_e halign_;
    justify_alignment_e jalign_;
    double scale_factor_;

    placements_list placements_;

    //ShieldSymbolizer
    bool has_marker_;
    marker_info_ptr marker_;
    box2d<double> marker_box_;
    bool marker_unlocked_;
    pixel_position marker_displacement_;
};

typedef boost::shared_ptr<placement_finder_ng> placement_finder_ng_ptr;

}//ns mapnik

#endif // PLACEMENT_FINDER_NG_HPP