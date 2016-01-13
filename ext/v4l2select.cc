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
#include "rubyinc.hh"
#include "v4l2select.hh"

V4L2Select::V4L2Select(void) throw (Error):
  m_rbArray(rb_ary_new()), m_rbSelection(Qnil)
{
}

V4L2Select::~V4L2Select(void)
{
}

void V4L2Select::add( unsigned int coding, unsigned int width, unsigned int height )
{
  rb_ary_push( m_rbArray, rb_ary_new3( 3, INT2NUM( coding ), INT2NUM( width ),
                                       INT2NUM( height ) ) );
}

static VALUE yield( VALUE arg )
{
  return rb_yield( arg );
}

void V4L2Select::make(void) throw (Error)
{
  int error;
  m_rbSelection = rb_protect( yield, m_rbArray, &error );
  if ( error ) {
    VALUE rbError = rb_funcall( rb_gv_get( "$!" ), rb_intern( "message" ), 0 );
    ERRORMACRO( false, Error, , "Error in block to \"V4L2Input.new\": "
                << StringValuePtr( rbError ) );
  };
  ERRORMACRO( TYPE( m_rbSelection ) == T_ARRAY, Error, , "Block must return a value of "
              "type 'Array'" );
}

unsigned int V4L2Select::coding(void)
{
  return NUM2INT(rb_ary_entry(m_rbSelection, 0));
}

unsigned int V4L2Select::width(void)
{
  return NUM2INT(rb_ary_entry(m_rbSelection, 1));
}

unsigned int V4L2Select::height(void)
{
  return NUM2INT(rb_ary_entry(m_rbSelection, 2));
}

VALUE V4L2Select::wrapRescue( VALUE rbValue )
{
  return rbValue;
}

