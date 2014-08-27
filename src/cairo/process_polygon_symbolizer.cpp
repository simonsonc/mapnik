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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/renderer_common/process_polygon_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

namespace mapnik
{


template <typename T>
void cairo_renderer<T>::process(polygon_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using conv_types = boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag>;
    using vertex_converter_type = vertex_converter<box2d<double>, cairo_context, polygon_symbolizer,
                                                   CoordTransform, proj_transform, agg::trans_affine,
                                                   conv_types, feature_impl>;

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);

    render_polygon_symbolizer<vertex_converter_type>(
        sym, feature, prj_trans, common_, common_.query_extent_, context_,
        [&](color const &fill, double opacity) {
            context_.set_color(fill, opacity);
            // fill polygon
            context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
            context_.fill();
        });
}

template void cairo_renderer<cairo_ptr>::process(polygon_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
