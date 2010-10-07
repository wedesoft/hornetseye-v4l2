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
#include <cassert>
#include "error.hh"

inline void checkType( VALUE rbValue, VALUE rbClass )
{
  ERRORMACRO( rb_funcall( rbValue, rb_intern( "kind_of?" ), 1, rbClass ) ==
              Qtrue, Error, ,
              "Argument must be of class \"" << rb_class2name( rbClass )
              << "\"." );
}

inline void checkStruct( VALUE rbValue, VALUE rbClass )
{
  Check_Type( rbValue, T_DATA );
  checkType( rbValue, rbClass );
}

