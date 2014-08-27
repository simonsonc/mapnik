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
#include <mapnik/make_unique.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
// mapnik symbolizer generics
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <cmath>

namespace mapnik
{

template <typename T>
void cairo_renderer<T>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using path_type = coord_transform<CoordTransform,geometry_type>;
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    mapnik::color fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128,128,128));
    double opacity = get<double>(sym, keys::fill_opacity, feature, common_.vars_, 1.0);
    double height = get<double>(sym, keys::height, feature, common_.vars_, 0.0);

    context_.set_operator(comp_op);

    render_building_symbolizer(
        feature, height,
        [&](geometry_type &faces) {
            path_type faces_path(common_.t_, faces, prj_trans);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8 / 255.0,
                               fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.add_path(faces_path);
            context_.fill();
        },
        [&](geometry_type &frame) {
            path_type path(common_.t_, frame, prj_trans);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8/255.0,
                              fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.set_line_width(common_.scale_factor_);
            context_.add_path(path);
            context_.stroke();
        },
        [&](geometry_type &roof) {
            path_type roof_path(common_.t_, roof, prj_trans);
            context_.set_color(fill, opacity);
            context_.add_path(roof_path);
            context_.fill();
        });
}

template void cairo_renderer<cairo_ptr>::process(building_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
