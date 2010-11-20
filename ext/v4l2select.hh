/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009, 2010 Jan Wedekind
   
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
#ifndef HORNETSEYE_V4L2SELECT_HH
#define HORNETSEYE_V4L2SELECT_HH

#include <boost/smart_ptr.hpp>
#include <errno.h>
#include "error.hh"

class V4L2Select
{
public:
  V4L2Select(void) throw (Error);
  virtual ~V4L2Select(void);
  void add( unsigned int coding, unsigned int width, unsigned int height );
  unsigned int make(void) throw (Error);
  unsigned int coding( unsigned int selection );
  unsigned int width( unsigned int selection );
  unsigned int height( unsigned int selection );
  static VALUE wrapRescue( VALUE rbValue );
protected:
  VALUE m_rbArray;
};

typedef boost::shared_ptr< V4L2Select > V4L2SelectPtr;

#endif

