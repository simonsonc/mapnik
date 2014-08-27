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
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/group/group_layout_manager.hpp>
#include <mapnik/group/group_symbolizer_helper.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/label_collision_detector.hpp>

#include <mapnik/geom_util.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>
#include <mapnik/renderer_common/process_group_symbolizer.hpp>

// agg
#include "agg_trans_affine.h"

namespace mapnik {

/**
 * Render a thunk which was frozen from a previous call to
 * extract_bboxes. We should now have a new offset at which
 * to render it, and the boxes themselves should already be
 * in the detector from the placement_finder.
 */
template <typename T0>
struct thunk_renderer : public util::static_visitor<>
{
    using renderer_type = grid_renderer<T0>;
    using buffer_type = typename renderer_type::buffer_type;
    using text_renderer_type = grid_text_renderer<buffer_type>;

    thunk_renderer(renderer_type &ren,
                   buffer_type &pixmap,
                   renderer_common &common,
                   feature_impl &feature,
                   pixel_position const &offset)
        : ren_(ren), pixmap_(pixmap), common_(common),
          feature_(feature), offset_(offset)
    {}

    void operator()(point_render_thunk const &thunk) const
    {
        ren_.render_marker(feature_, pixmap_.get_resolution(), offset_ + thunk.pos_,
                           *thunk.marker_, thunk.tr_, thunk.opacity_, thunk.comp_op_);
    }

    void operator()(text_render_thunk const &thunk) const
    {
        text_renderer_type ren(pixmap_, thunk.comp_op_, common_.scale_factor_);
        value_integer feature_id = feature_.id();

        render_offset_placements(
            thunk.placements_,
            offset_,
            [&] (glyph_positions_ptr glyphs)
            {
                if (glyphs->marker())
                {
                    ren_.render_marker(feature_, pixmap_.get_resolution(),
                                       glyphs->marker_pos(),
                                       *(glyphs->marker()->marker),
                                       glyphs->marker()->transform,
                                       thunk.opacity_, thunk.comp_op_);
                }
                ren.render(*glyphs, feature_id);
            });
    }

    template <typename T1>
    void operator()(T1 const &) const
    {
        // TODO: warning if unimplemented?
    }

private:
    renderer_type &ren_;
    buffer_type &pixmap_;
    renderer_common &common_;
    feature_impl &feature_;
    pixel_position offset_;
};

template <typename T>
void  grid_renderer<T>::process(group_symbolizer const& sym,
                                mapnik::feature_impl & feature,
                                proj_transform const& prj_trans)
{
    render_group_symbolizer(
        sym, feature, common_.vars_, prj_trans, common_.query_extent_, common_,
        [&](render_thunk_list const& thunks, pixel_position const& render_offset)
        {
            thunk_renderer<T> ren(*this, pixmap_, common_, feature, render_offset);
            for (render_thunk_ptr const& thunk : thunks)
            {
                util::apply_visitor(ren, *thunk);
            }
        });
}

template void grid_renderer<grid>::process(group_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
