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

#ifndef MAPNIK_VALUE_HASH_HPP
#define MAPNIK_VALUE_HASH_HPP

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/unicode.hpp>
// stl
#include <functional>

namespace mapnik { namespace detail {

template <class T>
inline void hash_combine(std::size_t & seed, T const& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct value_hasher: public util::static_visitor<std::size_t>
{
    std::size_t operator() (value_null val) const
    {
        return hash_value(val);
    }

    std::size_t operator() (value_unicode_string const& val) const
    {
        return hash_value(val);
    }

    template <class T>
    std::size_t operator()(T const& val) const
    {
        std::hash<T> hasher;
        return hasher(val);
    }
};

} // namespace  detail

template <typename T>
std::size_t hash_value(T const& val)
{
    std::size_t seed = util::apply_visitor(detail::value_hasher(), val);
    detail::hash_combine(seed, val.get_type_index());
    return seed;
}

} // namespace mapnik

#endif // MAPNIK_VALUE_HASH_HPP
