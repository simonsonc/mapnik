/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/rule.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/expression_string.hpp>

// all symbolizers
#include <mapnik/building_symbolizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/debug_symbolizer.hpp>

// stl
#include <memory>
#include <limits>

namespace mapnik
{

rule::rule()
    : name_(),
      min_scale_(0),
      max_scale_(std::numeric_limits<double>::infinity()),
      syms_(),
      filter_(std::make_shared<expr_node>(true)),
      else_filter_(false),
      also_filter_(false) {}

rule::rule(std::string const& name,
     double min_scale_denominator,
     double max_scale_denominator)
    : name_(name),
      min_scale_(min_scale_denominator),
      max_scale_(max_scale_denominator),
      syms_(),
      filter_(std::make_shared<expr_node>(true)),
      else_filter_(false),
      also_filter_(false)  {}

rule::rule(rule const& rhs)
    : name_(rhs.name_),
      min_scale_(rhs.min_scale_),
      max_scale_(rhs.max_scale_),
      syms_(rhs.syms_),
      filter_(std::make_shared<expr_node>(*rhs.filter_)),
      else_filter_(rhs.else_filter_),
      also_filter_(rhs.also_filter_)
{
}

rule& rule::operator=(rule const& rhs)
{
    rule tmp(rhs);
    swap(tmp);
    return *this;
}

bool rule::operator==(rule const& other)
{
    return  (this == &other);
}

void rule::swap(rule& rhs) throw()
{
    name_=rhs.name_;
    min_scale_=rhs.min_scale_;
    max_scale_=rhs.max_scale_;
    syms_=rhs.syms_;
    filter_=rhs.filter_;
    else_filter_=rhs.else_filter_;
    also_filter_=rhs.also_filter_;
}

void rule::set_max_scale(double scale)
{
    max_scale_=scale;
}

double rule::get_max_scale() const
{
    return max_scale_;
}

void rule::set_min_scale(double scale)
{
    min_scale_=scale;
}

double rule::get_min_scale() const
{
    return min_scale_;
}

void rule::set_name(std::string const& name)
{
    name_=name;
}

std::string const& rule::get_name() const
{
    return name_;
}

void rule::append(symbolizer const& sym)
{
    syms_.push_back(sym);
}

void rule::remove_at(size_t index)
{
    if (index < syms_.size())
    {
        syms_.erase(syms_.begin()+index);
    }
}

rule::symbolizers const& rule::get_symbolizers() const
{
    return syms_;
}

rule::symbolizers::const_iterator rule::begin() const
{
    return syms_.begin();
}

rule::symbolizers::const_iterator rule::end() const
{
    return syms_.end();
}

rule::symbolizers::iterator rule::begin()
{
    return syms_.begin();
}

rule::symbolizers::iterator rule::end()
{
    return syms_.end();
}

void rule::set_filter(expression_ptr const& filter)
{
    filter_=filter;
}

expression_ptr const& rule::get_filter() const
{
    return filter_;
}

void rule::set_else(bool else_filter)
{
    else_filter_=else_filter;
}

bool rule::has_else_filter() const
{
    return else_filter_;
}

void rule::set_also(bool also_filter)
{
    also_filter_=also_filter;
}

bool rule::has_also_filter() const
{
    return also_filter_;
}

}
