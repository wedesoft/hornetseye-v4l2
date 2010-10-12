/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009, 2010   Jan Wedekind

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
#include "frame.hh"

using namespace std;

Frame::Frame( const string &typecode, int width, int height, char *data ):
  m_frame( Qnil )
{
  VALUE mModule = rb_define_module( "Hornetseye" );
  VALUE cMalloc = rb_define_class_under( mModule, "Malloc", rb_cObject );
  VALUE cFrame = rb_define_class_under( mModule, "Frame", rb_cObject );
  VALUE rbSize = INT2NUM( storageSize( typecode, width, height ) );
  VALUE rbMemory;
  if ( data != NULL ) {
    rbMemory = Data_Wrap_Struct( cMalloc, 0, 0, (void *)data );
    rb_ivar_set( rbMemory, rb_intern( "@size" ), rbSize );
  } else
    rbMemory = rb_funcall( cMalloc, rb_intern( "new" ), 1, rbSize );
  m_frame = rb_funcall( cFrame, rb_intern( "import" ), 4,
                        rb_const_get( mModule, rb_intern( typecode.c_str() ) ),
                        INT2NUM( width ), INT2NUM( height ), rbMemory );
}

string Frame::typecode(void)
{
  VALUE rbString = rb_funcall( rb_funcall( m_frame, rb_intern( "typecode" ), 0 ),
                               rb_intern( "to_s" ), 0 );
  return StringValuePtr( rbString );
}

int Frame::width(void)
{
  return NUM2INT( rb_funcall( m_frame, rb_intern( "width" ), 0 ) );
}

int Frame::height(void)
{
  return NUM2INT( rb_funcall( m_frame, rb_intern( "height" ), 0 ) );
}

char *Frame::data(void)
{
  VALUE rbMemory = rb_funcall( m_frame, rb_intern( "memory" ), 0 );
  char *ptr;
  Data_Get_Struct( rbMemory, char, ptr );
  return ptr;
}

bool Frame::rgb(void)
{
  return rb_funcall( m_frame, rb_intern( "rgb?" ), 0 ) != Qfalse;
}

void Frame::markRubyMember(void)
{
  rb_gc_mark( m_frame );
}

int Frame::storageSize( const std::string &typecode, int width, int height )
{
  VALUE mModule = rb_define_module( "Hornetseye" );
  VALUE cFrame = rb_define_class_under( mModule, "Frame", rb_cObject );
  return NUM2INT( rb_funcall( cFrame, rb_intern( "storage_size" ), 3,
                              rb_const_get( mModule, rb_intern( typecode.c_str() ) ),
                              INT2NUM( width ), INT2NUM( height ) ) );
}
 
