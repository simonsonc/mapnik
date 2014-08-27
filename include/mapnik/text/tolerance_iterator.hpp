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
#ifndef MAPNIK_TOLERANCE_ITERATOR_HPP
#define MAPNIK_TOLERANCE_ITERATOR_HPP

//mapnik
#include <mapnik/debug.hpp>

namespace mapnik
{

class tolerance_iterator
{
public:
    tolerance_iterator(double label_position_tolerance, double spacing)
        : tolerance_(label_position_tolerance > 0 ?
                        label_position_tolerance : spacing/2.0),
          tolerance_delta_(std::max(1.0, tolerance_/100.0)),
          value_(0),
          initialized_(false),
          values_tried_(0)
    {
    }

    ~tolerance_iterator()
    {
        //std::cout << "values tried:" << values_tried_ << "\n";
    }

    double get() const
    {
        return -value_;
    }

    bool next()
    {
        ++values_tried_;
        if (values_tried_ > 255)
        {
            /* This point should not be reached during normal operation. But I can think of
             * cases where very bad spacing and or tolerance values are choosen and the
             * placement finder tries an excessive number of placements.
             * 255 is an arbitrarily chosen limit.
             */
            MAPNIK_LOG_WARN(placement_finder) << "Tried a huge number of placements. Please check "
                                                 "'label-position-tolerance' and 'spacing' parameters "
                                                 "of your TextSymbolizers.\n";
            return false;
        }
        if (!initialized_)
        {
            initialized_ = true;
            return true; //Always return value 0 as the first value.
        }
        if (value_ == 0)
        {
            value_ = tolerance_delta_;
            return true;
        }
        value_ = -value_;
        if (value_ > 0)
        {
            value_ += tolerance_delta_;
        }
        if (value_ > tolerance_)
        {
            return false;
        }
        return true;
    }
private:
    double tolerance_;
    double tolerance_delta_;
    double value_;
    bool initialized_;
    unsigned values_tried_;
};

}//ns mapnik

#endif // MAPNIK_TOLERANCE_ITERATOR_HPP
