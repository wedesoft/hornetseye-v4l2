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
#ifndef HORNETSEYE_RUBYTOOLS_HH
#define HORNETSEYE_RUBYTOOLS_HH

#include <complex>
#include "rubyinc.hh"

void checkType( VALUE rbValue, VALUE rbClass );

void checkStruct( VALUE rbValue, VALUE rbClass );

#define dataGetStruct(obj,klass,type,sval) { \
  checkStruct( obj, klass );               \
  Data_Get_Struct( obj, type, sval );      \
}

#include "rubytools.tcc"

#endif
