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

#ifndef MAPNIK_GRID_HPP
#define MAPNIK_GRID_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/global.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/conversions.hpp>

// stl
#include <cstdint>
#include <map>
#include <set>
#include <cmath>
#include <string>
#include <vector>

namespace mapnik
{

template <typename T>
class MAPNIK_DECL hit_grid
{
public:
    using value_type = T;
    using data_type = mapnik::ImageData<value_type>;
    using lookup_type = std::string;
    // mapping between pixel id and key
    using feature_key_type = std::map<value_type, lookup_type>;
    using feature_type = std::map<lookup_type, mapnik::feature_ptr>;
    static const value_type base_mask;

private:
    unsigned width_;
    unsigned height_;
    std::string key_;
    data_type data_;
    unsigned int resolution_;
    std::string id_name_;
    bool painted_;
    std::set<std::string> names_;
    feature_key_type f_keys_;
    feature_type features_;
    mapnik::context_ptr ctx_;

public:

    hit_grid(int width, int height, std::string const& key, unsigned int resolution);

    hit_grid(hit_grid<T> const& rhs);

    ~hit_grid() {}

    void clear();

    inline void painted(bool is_painted)
    {
        painted_ = is_painted;
    }

    inline bool painted() const
    {
        return painted_;
    }

    inline std::string const& key_name() const
    {
        return id_name_;
    }

    void add_feature(mapnik::feature_impl const& feature);

    inline void add_property_name(std::string const& name)
    {
        names_.insert(name);
    }

    inline std::set<std::string> const& property_names() const
    {
        return names_;
    }

    inline feature_type const& get_grid_features() const
    {
        return features_;
    }

    inline feature_key_type const& get_feature_keys() const
    {
        return f_keys_;
    }

    inline std::string const& get_key() const
    {
        return key_;
    }

    inline void set_key(std::string const& key)
    {
        key_ = key;
    }

    inline unsigned int get_resolution() const
    {
        return resolution_;
    }

    inline void set_resolution(unsigned int res)
    {
        resolution_ = res;
    }

    inline data_type const& data() const
    {
        return data_;
    }

    inline data_type& data()
    {
        return data_;
    }

    inline T const * raw_data() const
    {
        return data_.getData();
    }

    inline T* raw_data()
    {
        return data_.getData();
    }

    inline value_type const * getRow(unsigned row) const
    {
        return data_.getRow(row);
    }

    inline mapnik::grid_view get_view(unsigned x, unsigned y, unsigned w, unsigned h)
    {
        return mapnik::grid_view(x,y,w,h,
                                 data_,key_,id_name_,resolution_,names_,f_keys_,features_);
    }

private:

    inline bool checkBounds(unsigned x, unsigned y) const
    {
        return (x < width_ && y < height_);
    }

    hit_grid& operator=(const hit_grid&);

public:
    inline void setPixel(int x,int y,value_type feature_id)
    {
        if (checkBounds(x,y))
        {
            data_(x,y) = feature_id;
        }
    }
    inline unsigned width() const
    {
        return width_;
    }

    inline unsigned height() const
    {
        return height_;
    }

    inline void set_rectangle(value_type id,image_data_32 const& data,int x0,int y0)
    {
        box2d<int> ext0(0,0,width_,height_);
        box2d<int> ext1(x0,y0,x0+data.width(),y0+data.height());

        if (ext0.intersects(ext1))
        {
            box2d<int> box = ext0.intersect(ext1);
            for (int y = box.miny(); y < box.maxy(); ++y)
            {
                value_type* row_to =  data_.getRow(y);
                unsigned int const * row_from = data.getRow(y-y0);

                for (int x = box.minx(); x < box.maxx(); ++x)
                {
                    unsigned rgba = row_from[x-x0];
                    unsigned a = (rgba >> 24) & 0xff;
                    // if the pixel is more than a tenth
                    // opaque then burn in the feature id
                    if (a >= 25)
                    {
                        row_to[x] = id;
                    }
                }
            }
        }
    }

};

using grid = hit_grid<mapnik::value_integer>;

}
#endif //MAPNIK_GRID_HPP
