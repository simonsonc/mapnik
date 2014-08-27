/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include "geojson_datasource.hpp"
#include "geojson_featureset.hpp"

#include <fstream>
#include <algorithm>

// boost

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/json/feature_collection_grammar.hpp>
#include <mapnik/json/feature_collection_grammar_impl.hpp>
#include <mapnik/json/feature_grammar_impl.hpp>
#include <mapnik/json/geometry_grammar_impl.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(geojson_datasource)

struct attr_value_converter : public mapnik::util::static_visitor<mapnik::eAttributeType>
{
    mapnik::eAttributeType operator() (mapnik::value_integer) const
    {
        return mapnik::Integer;
    }

    mapnik::eAttributeType operator() (double) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (float) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (bool) const
    {
        return mapnik::Boolean;
    }

    mapnik::eAttributeType operator() (std::string const& ) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_unicode_string const&) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_null const& ) const
    {
        return mapnik::String;
    }
};

geojson_datasource::geojson_datasource(parameters const& params)
  : datasource(params),
    type_(datasource::Vector),
    desc_(geojson_datasource::name(),
          *params.get<std::string>("encoding","utf-8")),
    filename_(),
    inline_string_(),
    extent_(),
    features_(),
#if BOOST_VERSION >= 105600
    tree_()
#else
    tree_(16,1)
#endif
{
    boost::optional<std::string> inline_string = params.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");

        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }
    if (!inline_string_.empty())
    {
        std::istringstream in(inline_string_);
        parse_geojson(in);
    }
    else
    {
#if defined (_WINDOWS)
        std::ifstream in(mapnik::utf8_to_utf16(filename_),std::ios_base::in | std::ios_base::binary);
#else
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
#endif
        if (!in.is_open())
        {
            throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
        }
        parse_geojson(in);
        in.close();
    }
}

namespace {
using base_iterator_type = std::istreambuf_iterator<char>;
const mapnik::transcoder tr("utf8");
const mapnik::json::feature_collection_grammar<boost::spirit::multi_pass<base_iterator_type>,mapnik::feature_impl> fc_grammar(tr);
}

template <typename T>
void geojson_datasource::parse_geojson(T & stream)
{
    boost::spirit::multi_pass<base_iterator_type> begin =
        boost::spirit::make_default_multi_pass(base_iterator_type(stream));

    boost::spirit::multi_pass<base_iterator_type> end =
        boost::spirit::make_default_multi_pass(base_iterator_type());

    boost::spirit::standard_wide::space_type space;
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    bool result = boost::spirit::qi::phrase_parse(begin, end, (fc_grammar)(boost::phoenix::ref(ctx)), space, features_);
    if (!result)
    {
        if (!inline_string_.empty()) throw mapnik::datasource_exception("geojson_datasource: Failed parse GeoJSON file from in-memory string");
        else throw mapnik::datasource_exception("geojson_datasource: Failed parse GeoJSON file '" + filename_ + "'");
    }

    std::size_t geometry_index = 0;
    for (mapnik::feature_ptr const& f : features_)
    {
        mapnik::box2d<double> box = f->envelope();
        if (geometry_index == 0)
        {
            extent_ = box;
            for ( auto const& kv : *f)
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(std::get<0>(kv),
                                                                  mapnik::util::apply_visitor(attr_value_converter(),
                                                                                              std::get<1>(kv).base())));
            }
        }
        else
        {
            extent_.expand_to_include(box);
        }
#if BOOST_VERSION >= 105600
        tree_.insert(std::make_pair(box_type(point_type(box.minx(),box.miny()),point_type(box.maxx(),box.maxy())),geometry_index));
#else
        tree_.insert(box_type(point_type(box.minx(),box.miny()),point_type(box.maxx(),box.maxy())),geometry_index);
#endif
        ++geometry_index;
    }
}

geojson_datasource::~geojson_datasource() { }

const char * geojson_datasource::name()
{
    return "geojson";
}

boost::optional<mapnik::datasource::geometry_t> geojson_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::util::to_ds_type(features_[i]->paths(),result);
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource::Collection);
                return result;
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::datasource::datasource_t geojson_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> geojson_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor geojson_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr geojson_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& b = q.get_bbox();
    if (extent_.intersects(b))
    {
        box_type box(point_type(b.minx(),b.miny()),point_type(b.maxx(),b.maxy()));
#if BOOST_VERSION >= 105600
        geojson_featureset::array_type index_array;
        tree_.query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
        return std::make_shared<geojson_featureset>(features_, std::move(index_array));
#else
        return std::make_shared<geojson_featureset>(features_, tree_.find(box));
#endif
    }
    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr geojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    std::vector<mapnik::attribute_descriptor> const& desc = desc_.get_descriptors();
    std::vector<mapnik::attribute_descriptor>::const_iterator itr = desc.begin();
    std::vector<mapnik::attribute_descriptor>::const_iterator end = desc.end();
    for ( ;itr!=end;++itr)
    {
        q.add_property_name(itr->get_name());
    }
    return features(q);
}
