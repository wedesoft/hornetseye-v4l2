/* HornetsEye - Computer Vision with Ruby
   Copyright (C) 2006, 2007, 2008, 2009 Jan Wedekind
   
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
#ifndef HORNETSEYE_V4L2INPUT_HH
#define HORNETSEYE_V4L2INPUT_HH

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include "error.hh"
#include "frame.hh"
#include "v4l2select.hh"

class V4L2Input
{
public:
  V4L2Input( const std::string &device, V4L2SelectPtr select ) throw (Error);
  virtual ~V4L2Input(void);
  void close(void);
  FramePtr read(void) throw (Error);
  bool status(void) const;
  std::string inspect(void) const;
  int width(void) const { return m_format.fmt.pix.width; }
  int height(void) const { return m_format.fmt.pix.height; }
  bool hasFeature( int id ) throw (Error);
  int featureValue( unsigned int _id ) throw (Error);
  void setFeatureValue( unsigned int id, int value ) throw (Error);
  enum v4l2_ctrl_type featureType( unsigned int id ) throw (Error);
  std::string featureName( unsigned int id ) throw (Error);
  int featureMin( unsigned int id ) throw (Error);
  int featureMax( unsigned int id ) throw (Error);
  int featureStep( unsigned int id ) throw (Error);
  int featureDefaultValue( unsigned int id ) throw (Error);
  static VALUE cRubyClass;
  static VALUE registerRubyClass( VALUE module );
  static void deleteRubyObject( void *ptr );
  static VALUE wrapNew( VALUE rbClass, VALUE rbDevice );
  static VALUE wrapClose( VALUE rbSelf );
  static VALUE wrapRead( VALUE rbSelf );
  static VALUE wrapStatus( VALUE rbSelf );
  static VALUE wrapWidth( VALUE rbSelf );
  static VALUE wrapHeight( VALUE rbSelf );
  static VALUE wrapHasFeature( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureValue( VALUE rbSelf, VALUE rbId );
  static VALUE wrapSetFeatureValue( VALUE rbSelf, VALUE rbId,
                                    VALUE rbValue );
  static VALUE wrapFeatureType( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureName( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureMin( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureMax( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureStep( VALUE rbSelf, VALUE rbId );
  static VALUE wrapFeatureDefaultValue( VALUE rbSelf, VALUE rbId );
protected:
  int xioctl( int request, void *arg );
  std::string m_device;
  int m_fd;
  enum { IO_READ, IO_MMAP, IO_USERPTR } m_io;
  struct v4l2_format m_format;
  std::string m_typecode;
  struct v4l2_requestbuffers m_req;
  struct v4l2_buffer m_buf[2];
  void *m_map[2];
  void *m_user[2];
  bool m_frameUsed;
  struct v4l2_buffer m_frame;
};
  
typedef boost::shared_ptr< V4L2Input > V4L2InputPtr;

#endif

