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
#ifndef MAPNIK_TEXT_LINE_HPP
#define MAPNIK_TEXT_LINE_HPP

//mapnik
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
//stl
#include <vector>

namespace mapnik
{

/** This class stores all glyphs of a line in left to right order.
 *
 * It can be used for rendering but no text processing (like line breaking)
 * should be done!
 */
class text_line
{
public:
    text_line(unsigned first_char, unsigned last_char);

    typedef std::vector<glyph_info> glyph_vector;
    typedef glyph_vector::const_iterator const_iterator;
    /** Get glyph vector. */
    glyph_vector const& glyphs() const { return glyphs_; }
    /** Append glyph. */
    void add_glyph(glyph_info const& glyph, double scale_factor_);

    /** Preallocate memory. */
    void reserve(glyph_vector::size_type length);
    /** Iterator to first glyph. */
    const_iterator begin() const;
    /** Iterator beyond last glyph. */
    const_iterator end() const;

    /** Width of all glyphs including character spacing. */
    double width() const { return width_; }
    /** Real line height. For first line: max_char_height(), for all others: line_height(). */
    double height() const;

    /** Height of the tallest glyph in this line. */
    double max_char_height() const { return max_char_height_; }
    /** Called for each font/style to update the maximum height of this line. */
    void update_max_char_height(double max_char_height);
    /** Line height including line spacing. */
    double line_height() const { return line_height_; }

    /** Is this object is the first line of a multi-line text?
      * Used to exclude linespacing from first line's height. */
    void set_first_line(bool first_line);

    /** Index of first UTF-16 char. */
    unsigned first_char() const;
    /** Index of last UTF-16 char. */
    unsigned last_char() const;

    /** Number of glyphs. */
    unsigned size() const;
private:
    glyph_vector glyphs_;
    double line_height_; //Includes line spacing (returned by freetype)
    double max_char_height_; //Height of 'X' character of the largest font in this run. //TODO: Initialize this!
    double width_;
    unsigned first_char_;
    unsigned last_char_;
    bool first_line_;
};

} //namespace mapnik

#endif // MAPNIK_TEXT_LINE_HPP
