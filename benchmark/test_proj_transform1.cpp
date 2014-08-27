#include "bench_framework.hpp"
#include <mapnik/box2d.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>

class test : public benchmark::test_case
{
    std::string src_;
    std::string dest_;
    mapnik::box2d<double> from_;
    mapnik::box2d<double> to_;
    bool defer_proj4_init_;
public:
    test(mapnik::parameters const& params,
         std::string const& src,
         std::string const& dest,
         mapnik::box2d<double> const& from,
         mapnik::box2d<double> const& to,
         bool defer_proj)
     : test_case(params),
       src_(src),
       dest_(dest),
       from_(from),
       to_(to),
       defer_proj4_init_(defer_proj) {}
    bool validate() const
    {
        mapnik::projection src(src_,defer_proj4_init_);
        mapnik::projection dest(dest_,defer_proj4_init_);
        mapnik::proj_transform tr(src,dest);
        mapnik::box2d<double> bbox = from_;
        if (!tr.forward(bbox)) return false;
        return ((std::fabs(bbox.minx() - to_.minx()) < .5) &&
                (std::fabs(bbox.maxx() - to_.maxx()) < .5) &&
                (std::fabs(bbox.miny() - to_.miny()) < .5) &&
                (std::fabs(bbox.maxy() - to_.maxy()) < .5)
               );
    }
    void operator()() const
    {
        for (std::size_t i=0;i<iterations_;++i) {
            for (int i=-180;i<180;i=i+5)
            {
                for (int j=-85;j<85;j=j+5)
                {
                    mapnik::projection src(src_,defer_proj4_init_);
                    mapnik::projection dest(dest_,defer_proj4_init_);
                    mapnik::proj_transform tr(src,dest);
                    mapnik::box2d<double> box(i,j,i,j);
                    if (!tr.forward(box)) throw std::runtime_error("could not transform coords");
                }
            }
        }
    }
};

// echo -180 -60 | cs2cs -f "%.10f" +init=epsg:4326 +to +init=epsg:3857
int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    mapnik::box2d<double> from(-180,-80,180,80);
    mapnik::box2d<double> to(-20037508.3427892476,-15538711.0963092316,20037508.3427892476,15538711.0963092316);
    std::string from_str("+init=epsg:4326");
    std::string to_str("+init=epsg:3857");
    std::string from_str2("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    std::string to_str2("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over");
    test test_runner(params,
                     from_str,
                     to_str,
                     from,
                     to,
                     true);
    run(test_runner,"lonlat->merc epsg");
    test test_runner2(params,
                     from_str2,
                     to_str2,
                     from,
                     to,
                     true);
    run(test_runner2,"lonlat->merc literal");
    test test_runner3(params,
                     to_str,
                     from_str,
                     to,
                     from,
                     true);
    run(test_runner3,"merc->lonlat epsg");
    test test_runner4(params,
                     to_str2,
                     from_str2,
                     to,
                     from,
                     true);
    return run(test_runner4,"merc->lonlat literal");
}
