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

#ifndef MAPNIK_BINDINGS_PYTHON_ENUMERATION_WRAPPPER
#define MAPNIK_BINDINGS_PYTHON_ENUMERATION_WRAPPPER

// mapnik
#include <mapnik/symbolizer.hpp>

// boost
#include <boost/python.hpp>


namespace boost { namespace python {

    struct mapnik_enumeration_wrapper_to_python
    {
        static PyObject* convert(mapnik::enumeration_wrapper const& v)
        {
            return ::PyLong_FromLongLong(v.value); // FIXME: this is a temp hack!!
        }
    };

}}

#endif // MAPNIK_BINDINGS_PYTHON_ENUMERATION_WRAPPPER
