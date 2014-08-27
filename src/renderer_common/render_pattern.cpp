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
// mapnik
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"

namespace mapnik {

std::shared_ptr<image_data_32> render_pattern(rasterizer & ras,
                                              marker const& marker,
                                              agg::trans_affine const& tr,
                                              double opacity)
{
    using pixfmt = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
    agg::scanline_u8 sl;

    mapnik::box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box() * tr;
    mapnik::coord<double,2> c = bbox.center();
    agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
    mtx.translate(0.5 * bbox.width(), 0.5 * bbox.height());
    mtx = tr * mtx;

    std::shared_ptr<mapnik::image_data_32> image = std::make_shared<mapnik::image_data_32>(bbox.width(), bbox.height());
    agg::rendering_buffer buf(image->getBytes(), image->width(), image->height(), image->width() * 4);
    pixfmt pixf(buf);
    renderer_base renb(pixf);

    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage((*marker.get_vector_data())->source());
    mapnik::svg::svg_path_adapter svg_path(stl_storage);
    mapnik::svg::svg_renderer_agg<mapnik::svg::svg_path_adapter,
                                  agg::pod_bvector<mapnik::svg::path_attributes>,
                                  renderer_solid,
                                  agg::pixfmt_rgba32_pre > svg_renderer(svg_path,
                                                                        (*marker.get_vector_data())->attributes());

    svg_renderer.render(ras, sl, renb, mtx, opacity, bbox);
    return image;
}

} // namespace mapnik
