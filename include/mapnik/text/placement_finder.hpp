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
#ifndef MAPNIK_PLACEMENT_FINDER_HPP
#define MAPNIK_PLACEMENT_FINDER_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements_list.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/noncopyable.hpp>

namespace mapnik
{

class label_collision_detector4;
typedef label_collision_detector4 DetectorType;

class feature_impl;
class vertex_cache;

class placement_finder : mapnik::noncopyable
{
public:
    placement_finder(feature_impl const& feature,
                        DetectorType & detector,
                        box2d<double> const& extent,
                        text_placement_info_ptr placement_info,
                        face_manager_freetype & font_manager,
                        double scale_factor);

    /** Try to place a single label at the given point. */
    bool find_point_placement(pixel_position const& pos);
    /** Iterate over the given path, placing line-following labels or point labels with respect to label_spacing. */
    template <typename T>
    bool find_line_placements(T & path, bool points);
    /** Try next position alternative from placement_info. */
    bool next_position();

    placements_list const& placements() const { return placements_; }

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
    /** Maps upright==auto, left_only and right_only to left,right to simplify processing.
        angle = angle of at start of line (to estimate best option for upright==auto) */
    text_upright_e simplify_upright(text_upright_e upright, double angle) const;
    box2d<double> get_bbox(glyph_info const& glyph, pixel_position const& pos, rotation const& rot);
    feature_impl const& feature_;
    DetectorType &detector_;
    box2d<double> const& extent_;
    // Precalculated values for maximum performance
    rotation orientation_;
    text_layout layout_;
    text_placement_info_ptr info_;
    bool valid_;

    vertical_alignment_e valign_;
    /** Horizontal alignment for point placements. */
    horizontal_alignment_e halign_point_;
    /** Horizontal alignment for line placements. */
    horizontal_alignment_e halign_line_;
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

}//ns mapnik

#endif // PLACEMENT_FINDER_HPP
