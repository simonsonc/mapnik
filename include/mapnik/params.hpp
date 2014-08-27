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

#ifndef MAPNIK_PARAMS_HPP
#define MAPNIK_PARAMS_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/util/variant.hpp>
// boost
#include <boost/optional.hpp>

// stl
#include <string>
#include <map>

namespace mapnik
{

// fwd declare
class boolean_type;

using value_holder_base = util::variant<value_null,value_integer,value_double,std::string>;

struct value_holder : value_holder_base
{
    // default
    value_holder()
        : value_holder_base() {}

    // copy
    value_holder(const char* val)
        : value_holder_base(val) {}

    template <typename T>
    value_holder(T const& obj)
        : value_holder_base(typename detail::mapnik_value_type<T>::type(obj))
    {}

    // move
    template <typename T>
    value_holder(T && obj) noexcept
        : value_holder_base(std::move(obj)) {}
};

using parameter = std::pair<std::string, value_holder>;
using param_map = std::map<std::string, value_holder>;

class MAPNIK_DECL parameters : public param_map
{
public:
    parameters() {}
    template <typename T>
    boost::optional<T> get(std::string const& key) const;
    template <typename T>
    boost::optional<T> get(std::string const& key, T const& default_opt_value) const;

};

#ifdef _MSC_VER
template MAPNIK_DECL
boost::optional<std::string> parameters::get(std::string const& key) const;
template MAPNIK_DECL
boost::optional<std::string> parameters::get(std::string const& key,
                                 std::string const& default_opt_value) const;
template MAPNIK_DECL
boost::optional<value_double> parameters::get(std::string const& key) const;
template MAPNIK_DECL
boost::optional<value_double> parameters::get(std::string const& key,
                                  value_double const& default_opt_value) const;

template MAPNIK_DECL
boost::optional<value_integer> parameters::get(std::string const& key) const;
template MAPNIK_DECL
boost::optional<value_integer> parameters::get(std::string const& key,
                                   value_integer const& default_opt_value) const;

template MAPNIK_DECL
boost::optional<mapnik::boolean_type> parameters::get(std::string const& key) const;
template MAPNIK_DECL
boost::optional<mapnik::boolean_type> parameters::get(std::string const& key,
                                         mapnik::boolean_type const& default_opt_value) const;
#endif

}

#endif // MAPNIK_PARAMS_HPP
