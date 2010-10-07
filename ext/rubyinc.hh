/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007   Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef HORNETSEYE_RUBYINC_HH
#define HORNETSEYE_RUBYINC_HH

#ifdef RSHIFT
#undef RSHIFT
#endif

#define gettimeofday rubygettimeofday
#define timezone rubygettimezone
#include <ruby.h>
// #include <version.h>
#undef timezone
#undef gettimeofday
#ifdef read
#undef read
#endif
#ifdef write
#undef write
#endif
#ifdef RGB
#undef RGB
#endif

#ifndef RUBY_VERSION_NUMBER
#define RUBY_VERSION_NUMBER ( RUBY_VERSION_MAJOR * 10000 + \
                              RUBY_VERSION_MINOR * 100 + \
                              RUBY_VERSION_TEENY )
#endif

#ifndef RUBY_METHOD_FUNC
#define RUBY_METHOD_FUNC(func) ((VALUE (*)(ANYARGS))func)
#endif

#ifndef xfree
#define xfree free
#endif

#endif

