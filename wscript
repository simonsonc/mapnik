# vim: set filetype=python :
from waflib import Utils

APPNAME="mapnik"
VERSION="2.1.0"

def options(opt):
    opt.load('compiler_c compiler_cxx')
    opt.load('python')
    opt.load('waf_unit_test')
    opt.load('boost', tooldir='waftools')
    opt.recurse('demo/viewer')

def configure(conf):
    conf.load('compiler_c compiler_cxx')
    conf.load('python')
    conf.load('waf_unit_test')
    conf.load('boost', tooldir='waftools')

    conf.env.MAPNIK_VERSION_STRING = VERSION

    # python bindings
    conf.check_python_version((2,2,0))
    conf.check_python_headers()
    '''
    conf.check(lib="boost_python", uselib_store="BOOST_PYTHON")
    '''
    conf.check_boost(lib="python", uselib_store="BOOST_PYTHON")

    # required libraries
    conf.check(header_name="zlib.h")
    conf.check(header_name="ltdl.h", lib="ltdl", uselib_store="LTDL")
    conf.check(header_name="png.h", lib="png", uselib_store="PNG")
    conf.check(header_name="proj_api.h", lib="proj", uselib_store="PROJ")
    conf.check(header_name="tiff.h", lib="tiff", uselib_store="TIFF")
    conf.check(header_name="stdlib.h stdio.h jpeglib.h", lib="jpeg", uselib_store="JPEG", define_name="HAVE_JPEG")
    conf.check(header_name="unicode/uversion.h", lib="icuuc", uselib_store="ICU")
    conf.check_cfg(package="freetype2", args="--cflags --libs", uselib_store="FREETYPE")
    conf.check_cfg(package="libxml-2.0", args="--cflags --libs", uselib_store="LIBXML2")
    conf.check_cfg(package="libcurl", args="--cflags --libs", uselib_store="LIBCURL")
    conf.check_cfg(package="harfbuzz", args="--cflags --libs", uselib_store="HARFBUZZ")
    conf.check_cfg(package="harfbuzz-icu", args="--cflags --libs", uselib_store="HARFBUZZ_ICU")

    # boost libraries
    '''
    conf.check(header_name="boost/regex.hpp", lib="boost_regex", uselib_store="BOOST_REGEX")
    conf.check(header_name="boost/system/config.hpp", lib="boost_system", uselib_store="BOOST_SYSTEM")
    conf.check(header_name="boost/filesystem.hpp", lib=["boost_filesystem", "boost_system"], uselib_store="BOOST_FILESYSTEM")
    conf.check(header_name="boost/thread.hpp", lib="boost_thread", uselib_store="BOOST_THREAD")
    conf.check(header_name="boost/program_options.hpp", lib="boost_program_options", uselib_store="BOOST_PROGRAM_OPTIONS")
    '''
    conf.check_boost(lib='regex', uselib_store="BOOST_REGEX")
    conf.check_boost(lib='system', uselib_store="BOOST_SYSTEM")
    conf.check_boost(lib='filesystem system', uselib_store="BOOST_FILESYSTEM")
    conf.check_boost(lib='thread system', uselib_store="BOOST_THREAD")
    conf.check_boost(lib='program_options', uselib_store="BOOST_PROGRAM_OPTIONS")

    # cairo optional renderer
    conf.check_cfg(package="cairo", args="--cflags --libs", uselib_store="CAIRO")
    conf.check_cfg(package="cairomm-1.0", args="--cflags --libs", uselib_store="CAIROMM")
    conf.check_cfg(package="pycairo", args="--cflags --libs", uselib_store="PYCAIRO")

    # GDAL
    conf.check_cfg(path="gdal-config", args="--cflags", package='', uselib_store="GDAL")
    conf.check_cfg(path="gdal-config", args="--libs", package='', uselib_store="GDAL")
    # --ogr-enabled says "yes" or "no"
    #conf.check_cfg(path="gdal-config", args="--cflags --libs --ogr-enabled", package='', uselib_store='GDAL_OGR')

    # (only for rasterlite plugin)
    conf.check_cfg(package="rasterlite", args="--cflags --libs", uselib_store="RASTERLITE")

    # (only for sqlite plugin)
    conf.check_cfg(package="sqlite3", args="--cflags --libs", uselib_store="SQLITE")

    # (only for postgres plugin and test application)
    conf.env.INCLUDES_POSTGRES = Utils.to_list(conf.cmd_and_log("pg_config --includedir"))
    conf.env.LIBPATH_POSTGRES = Utils.to_list(conf.cmd_and_log("pg_config --libdir"))
    conf.env.LIB_POSTGRES = ['pq']

    # recurse
    conf.recurse('demo/viewer')

    # common variables
    conf.env.MAPNIK_LIBDIR = conf.env.LIBDIR + '/mapnik'

def build(bld):
    def build_plugin(name, **kw):
        env = bld.env.derive()
        env['cxxshlib_PATTERN'] = '%s.input'
        kw.update(
            name="plugin-" + name,
            target=name,
            install_path='${MAPNIK_LIBDIR}/input',
            env=env)
        bld.shlib(**kw)
    bld.plugin = build_plugin

    bld.recurse('deps/clipper')
    bld.recurse('deps/agg')
    bld.recurse('src')
    bld.recurse('include')
    bld.recurse('fonts')

    bld.recurse('plugins/input/csv')
    bld.recurse('plugins/input/gdal')
    bld.recurse('plugins/input/geojson')
    #bld.recurse('plugins/input/occi')
    bld.recurse('plugins/input/ogr')
    bld.recurse('plugins/input/osm')
    bld.recurse('plugins/input/postgis')
    bld.recurse('plugins/input/python')
    bld.recurse('plugins/input/raster')
    bld.recurse('plugins/input/rasterlite')
    bld.recurse('plugins/input/shape')
    bld.recurse('plugins/input/sqlite')
    #bld.recurse('plugins/input/templates/helloworld')

    bld.recurse('bindings/python')

    bld.recurse('tests/cpp_tests')
    #bld.recurse('tests/cpp_tests/svg_renderer_tests')

    bld.recurse('demo/c++')
    bld.recurse('demo/viewer')

    bld.recurse('utils/mapnik-config')
    bld.recurse('utils/ogrindex')
    bld.recurse('utils/performance')
    bld.recurse('utils/pgsql2sqlite')
    bld.recurse('utils/shapeindex')
    bld.recurse('utils/svg2png')
    bld.recurse('utils/upgrade_map_xml')

    from waflib.Tools import waf_unit_test
    bld.add_post_fun(waf_unit_test.summary)
