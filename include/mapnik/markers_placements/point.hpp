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

#ifndef MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
#define MAPNIK_MARKERS_PLACEMENTS_POINT_HPP

#include <mapnik/geom_util.hpp>

#include "agg_basics.h"
#include "agg_trans_affine.h"

namespace mapnik {

struct markers_placement_params
{
    box2d<double> const& size;
    agg::trans_affine const& tr;
    double spacing;
    double max_error;
    bool allow_overlap;
};

template <typename Locator, typename Detector>
class markers_point_placement
{
public:
    markers_point_placement(Locator &locator, Detector &detector, markers_placement_params const& params)
        : locator_(locator),
          detector_(detector),
          params_(params),
          done_(false)
    {
        rewind();
    }

    // Start again at first marker. Returns the same list of markers only works when they were NOT added to the detector.
    void rewind()
    {
        locator_.rewind(0);
        done_ = false;
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (done_)
        {
            return false;
        }

        if (locator_.type() == mapnik::geometry_type::types::LineString)
        {
            if (!label::middle_point(locator_, x, y))
            {
                done_ = true;
                return false;
            }
        }
        else
        {
            if (!label::centroid(locator_, x, y))
            {
                done_ = true;
                return false;
            }
        }

        angle = 0;
        box2d<double> box = perform_transform(angle, x, y);

        if (!params_.allow_overlap && !detector_.has_placement(box))
        {
            return false;
        }

        if (!ignore_placement)
        {
            detector_.insert(box);
        }

        done_ = true;
        return true;
    }

protected:
    Locator &locator_;
    Detector &detector_;
    markers_placement_params const& params_;
    bool done_;

    // Rotates the size_ box and translates the position.
    box2d<double> perform_transform(double angle, double dx, double dy)
    {
        double x1 = params_.size.minx();
        double x2 = params_.size.maxx();
        double y1 = params_.size.miny();
        double y2 = params_.size.maxy();
        agg::trans_affine tr = params_.tr * agg::trans_affine_rotation(angle).translate(dx, dy);
        double xA = x1, yA = y1,
               xB = x2, yB = y1,
               xC = x2, yC = y2,
               xD = x1, yD = y2;
        tr.transform(&xA, &yA);
        tr.transform(&xB, &yB);
        tr.transform(&xC, &yC);
        tr.transform(&xD, &yD);
        box2d<double> result(xA, yA, xC, yC);
        result.expand_to_include(xB, yB);
        result.expand_to_include(xD, yD);
        return result;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
