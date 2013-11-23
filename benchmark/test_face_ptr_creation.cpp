#include "bench_framework.hpp"
#include <mapnik/font_engine_freetype.hpp>
#include <boost/format.hpp>

class test : public benchmark::test_case
{
public:
    test(mapnik::parameters const& params)
     : test_case(params) {}
    bool validate() const
    {
        return true;
    }
    void operator()() const
    {
        mapnik::freetype_engine engine;
        unsigned long count = 0;
        for (unsigned i=0;i<iterations_;++i)
        {
            for ( std::string const& name : mapnik::freetype_engine::face_names())
            {
                mapnik::face_ptr f = engine.create_face(name);
                if (f) ++count;
            }
        }
    }
};

int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    bool success = mapnik::freetype_engine::register_fonts("./fonts", true);
    if (!success) {
       std::clog << "warning, did not register any new fonts!\n";
       return -1;
    } 
    std::size_t face_count = mapnik::freetype_engine::face_names().size();
    test test_runner(params);
    return run(test_runner,(boost::format("font_engine: creating %ld faces") % (face_count * 1000 * 10)).str());
}

