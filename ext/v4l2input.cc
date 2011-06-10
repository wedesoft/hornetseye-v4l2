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
#include <boost/array.hpp>
#ifndef NDEBUG
#include <iostream>
#endif
#include <malloc.h>
#include <map>
#include <sstream>
#include "rubytools.hh"
#include "v4l2input.hh"

// http://v4l2spec.bytesex.org/spec-single/v4l2.html

using namespace boost;
using namespace std;

VALUE V4L2Input::cRubyClass = Qnil;

V4L2Input::V4L2Input( const std::string &device, V4L2SelectPtr select ) throw (Error):
  m_device(device), m_fd(-1), m_io(IO_READ), m_frameUsed(false)
{
  m_map[ 0 ] = MAP_FAILED;
  m_map[ 1 ] = MAP_FAILED;
  m_user[ 0 ] = NULL;
  m_user[ 1 ] = NULL;
  struct stat st;
  ERRORMACRO( stat( device.c_str(), &st ) == 0, Error, ,
              "Couldn't read file-attributes of \"" << m_device << "\": "
              << strerror( errno ) );
  ERRORMACRO( S_ISCHR(st.st_mode), Error, ,
              '"' << m_device << "\" is not a device" );
  m_fd = open( m_device.c_str(), O_RDWR, 0 );
  ERRORMACRO( m_fd != -1, Error, ,
              "Could not open device \"" << m_device << "\": "
              << strerror( errno ) );
  try {
    struct v4l2_capability capability;
    ERRORMACRO( xioctl( VIDIOC_QUERYCAP, &capability ) == 0, Error, ,
                "Error requesting video capabilities of device \""
                << m_device << "\": " << strerror( errno ) );
#ifndef NDEBUG
    cerr << capability.card << " (" << capability.driver << ')' << endl;
#endif
    ERRORMACRO( capability.capabilities & V4L2_CAP_VIDEO_CAPTURE != 0,
                Error, , m_device << " is no video capture device" );
    unsigned int formatIndex = 0;
    while ( true ) {
      struct v4l2_fmtdesc format;
      memset( &format, 0, sizeof(format) );
      format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      format.index = formatIndex++;
      int r = xioctl( VIDIOC_ENUM_FMT, &format );
#ifndef NDEBUG
      cerr << "Requesting format " << (formatIndex - 1) << " with result " << r << endl;
#endif
      if ( r != 0 ) break;
      unsigned int frameSizeIndex = 0;
      while ( true ) {
        struct v4l2_frmsizeenum pix;
        memset( &pix, 0, sizeof(pix) );
        pix.pixel_format = format.pixelformat;
        pix.index = frameSizeIndex++;
        int r = xioctl( VIDIOC_ENUM_FRAMESIZES, &pix );
#ifndef NDEBUG
        cerr << "Requesting frame size " << (frameSizeIndex - 1) << " for format "
             << (formatIndex - 1) << " with result " << r << endl;
#endif
        if ( r != 0 ) break;
        if ( pix.type == V4L2_FRMSIZE_TYPE_DISCRETE )
          select->add( format.pixelformat, pix.discrete.width, pix.discrete.height );
        else if ( pix.type == V4L2_FRMSIZE_TYPE_STEPWISE ) {
          unsigned int
            w = pix.stepwise.min_width,
            h = pix.stepwise.min_height;
          while ( w <= pix.stepwise.max_width && h <= pix.stepwise.max_height ) {
            select->add( format.pixelformat, w, h );
            w += pix.stepwise.step_width;
            h += pix.stepwise.step_height;
          };
        } else {
          select->add( format.pixelformat,
                       pix.stepwise.max_width, pix.stepwise.max_height );
        };
      };
    };
    select->make();
    unsigned int coding = select->coding();
#ifndef NDEBUG
    cerr << "coding = " << select->coding() << endl
         << "w = " << select->width() << endl
         << "h = " << select->height() << endl;
#endif
    m_format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_format.fmt.pix.field       = V4L2_FIELD_NONE;
    m_format.fmt.pix.width       = select->width();
    m_format.fmt.pix.height      = select->height();
    m_format.fmt.pix.pixelformat = coding;
    m_format.fmt.pix.field       = V4L2_FIELD_SEQ_TB;
    ERRORMACRO( xioctl( VIDIOC_S_FMT, &m_format ) == 0, Error, ,
                 "Error switching device \"" << m_device << "\" to selected format" );
    switch ( coding ) {
    case V4L2_PIX_FMT_UYVY:
      m_typecode = "UYVY";
      break;
    case V4L2_PIX_FMT_YUYV:
      m_typecode = "YUY2";
      break;
    case V4L2_PIX_FMT_YUV420:
      m_typecode = "I420";
      break;
    case V4L2_PIX_FMT_GREY:
      m_typecode = "UBYTE";
      break;
    case V4L2_PIX_FMT_RGB24:
      m_typecode = "UBYTERGB";
      break;
    case V4L2_PIX_FMT_BGR24:
      m_typecode = "BGR";
      break;
    case V4L2_PIX_FMT_MJPEG:
      m_typecode = "MJPG";
      break;
    default:
      ERRORMACRO( false, Error, , "Conversion for V4L2 colorspace " << coding
                  << " not implemented yet" );
    };
    if ( capability.capabilities & V4L2_CAP_STREAMING ) {
      try {
#ifndef NDEBUG
        cerr << "Trying memory mapped I/O." << endl;
#endif
        // m_format.fmt.pix.sizeimage;
        memset( &m_req, 0, sizeof(m_req) );
        m_req.count = 2;
        m_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_req.memory = V4L2_MEMORY_MMAP;
        ERRORMACRO( xioctl( VIDIOC_REQBUFS, &m_req ) == 0, Error, ,
                    "Memory mapped I/O not supported for device \""
                    << m_device << '"' );
        ERRORMACRO( m_req.count >= 2, Error, ,
                    "Insufficient buffer memory on device \""
                    << m_device << '"' );
        for ( int i=0; i<2; i++ ) {
          memset( &m_buf[i], 0, sizeof(struct v4l2_buffer) );
          m_buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          m_buf[i].memory = V4L2_MEMORY_MMAP;
          m_buf[i].index = i;
          ERRORMACRO( xioctl( VIDIOC_QUERYBUF, &m_buf[i] ) == 0, Error, ,
                      "Error querying buffer " << i << " for device \""
                      << m_device << '"' );
          m_map[i] = mmap( NULL, m_buf[i].length, PROT_READ | PROT_WRITE,
                           MAP_SHARED, m_fd, m_buf[i].m.offset );
          ERRORMACRO( m_map[ i ] != MAP_FAILED, Error, ,
                      "Error mapping capture buffer " << i
                      << " for device \"" << m_device << '"' );
        };
        for ( int i=0; i<2; i++ ) {
          ERRORMACRO( xioctl( VIDIOC_QBUF, &m_buf[i] ) == 0, Error, ,
                      "Error enqueuing buffer " << i
                      << " for device \"" << m_device << '"' );
        };
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ERRORMACRO( xioctl( VIDIOC_STREAMON, &type ) == 0, Error, ,
                    "Error starting memory-mapped capture process" );
        m_io = IO_MMAP;
#ifndef NDEBUG
        cerr << "Using memory mapped I/O." << endl;
#endif
      } catch ( Error &e ) {
        for ( int i=0; i<2; i++ )
          if ( m_map[ i ] != MAP_FAILED ) {
            munmap( m_map[ i ], m_buf[ i ].length );
            m_map[ i ] = MAP_FAILED;
          };
#ifndef NDEBUG
        cerr << "Trying user pointer I/O." << endl;
#endif
        // m_format.fmt.pix.sizeimage;
        memset( &m_req, 0, sizeof(m_req) );
        m_req.count = 2;
        m_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_req.memory = V4L2_MEMORY_USERPTR;
        ERRORMACRO( xioctl( VIDIOC_REQBUFS, &m_req ) == 0, Error, ,
                    "User pointer I/O not supported for device \""
                    << m_device << "\" after memory-mapped I/O failed ("
                    << e.what() << ')' );
        int
          pageSize = getpagesize(),
          bufferSize = ( m_format.fmt.pix.sizeimage +
                         pageSize - 1 ) & ~( pageSize - 1 );
        m_user[0] = memalign( pageSize, bufferSize );
        m_user[1] = memalign( pageSize, bufferSize );
        ERRORMACRO( m_user[0] != NULL && m_user[1] != NULL, Error, ,
                    "Insufficient memory for user pointer I/O after "
                    "memory-mapped I/O failed (" << e.what() << ')' );
        for ( int i=0; i<2; i++ ) {
          memset( &m_buf[i], 0, sizeof(struct v4l2_buffer) );
          m_buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          m_buf[i].memory = V4L2_MEMORY_USERPTR;
          m_buf[i].index = i;
          m_buf[i].m.userptr = (unsigned long)m_user[i];
          m_buf[i].length = bufferSize;
        };
        for ( int i=0; i<2; i++ ) {
          ERRORMACRO( xioctl( VIDIOC_QBUF, &m_buf[i] ) == 0, Error, ,
                    "Error enqueuing user memory buffer " << i
                    << " for device \"" << m_device << "\" after "
                    "memory-mapped I/O failed (" << e.what() << ')' );
        };
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ERRORMACRO( xioctl( VIDIOC_STREAMON, &type ) == 0, Error, ,
                    "Error starting user pointer capture process after "
                    "memory-mapped I/O failed (" << e.what() << ')' );
        m_io = IO_USERPTR;
#ifndef NDEBUG
        cerr << "Using user pointer I/O." << endl;
#endif
      };
    } else if ( capability.capabilities & V4L2_CAP_READWRITE ) {
      m_io = IO_READ;
#ifndef NDEBUG
      cerr << "Using device reads." << endl;
#endif
      // Do nothing.
    } else {
      ERRORMACRO( false, Error, ,
                  "Camera neither supports streaming nor reading from "
                  "device" );
    };
  } catch ( Error &e ) {
    close();
    throw e;
  };
}

V4L2Input::~V4L2Input(void)
{
  close();
}

void V4L2Input::close(void)
{
  if ( m_io == IO_MMAP || m_io == IO_USERPTR ) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl( VIDIOC_STREAMOFF, &type );
  };
  for ( int i=0; i<2; i++ )
    if ( m_map[ i ] != MAP_FAILED ) {
      munmap( m_map[ i ], m_buf[ i ].length );
      m_map[ i ] = MAP_FAILED;
    };
  for ( int i=0; i<2; i++ )
    if ( m_user[ i ] != NULL ) {
      free( m_user[ i ] );
      m_user[ i ] = NULL;
    };
  if ( m_fd != -1 ) {
    ::close( m_fd );
    m_fd = -1;
  };
}

string V4L2Input::inspect(void) const
{
  ostringstream s;
  s << "V4L2Input( '" << m_device << "' )";
  return s.str();
}

bool V4L2Input::status(void) const
{
  return m_fd != -1;
}

bool V4L2Input::hasFeature( int id ) throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  bool retVal;
  if ( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0 )
    retVal = ( queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ) == 0;
  else
    retVal = false;
  return retVal;
}

int V4L2Input::featureValue( unsigned int id ) throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_control control;
  memset( &control, 0, sizeof(control) );
  control.id = id;
  ERRORMACRO( xioctl( VIDIOC_G_CTRL, &control ) == 0, Error, ,
              "Error reading value of control " << id );
  return control.value;
}

void V4L2Input::setFeatureValue( unsigned int id, int value )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_control control;
  memset( &control, 0, sizeof(control) );
  control.id = id;
  control.value = value;
  ERRORMACRO( xioctl( VIDIOC_S_CTRL, &control ) == 0, Error, ,
              "Error setting value of control " << id );
}

enum v4l2_ctrl_type V4L2Input::featureType( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return queryctrl.type;
}

string V4L2Input::featureName( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return string( (const char *)queryctrl.name );
}

int V4L2Input::featureMin( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return queryctrl.minimum;
}

int V4L2Input::featureMax( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return queryctrl.maximum;
}

int V4L2Input::featureStep( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return queryctrl.step;
}

int V4L2Input::featureDefaultValue( unsigned int id )
  throw (Error)
{
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  struct v4l2_queryctrl queryctrl;
  memset( &queryctrl, 0, sizeof(queryctrl) );
  queryctrl.id = id;
  ERRORMACRO( xioctl( VIDIOC_QUERYCTRL, &queryctrl ) == 0, Error, ,
              "Error requesting information about control " << id );
  return queryctrl.default_value;
}

FramePtr V4L2Input::read(void) throw (Error)
{
  FramePtr retVal;
  ERRORMACRO( m_fd != -1, Error, ,
              "Device \"" << m_device << "\" is not open. "
              "Did you call \"close\" before?" );
  int size;
  // Use custom-size for MJPEG frames.
  if ( m_typecode == "MJPG" )
    size = m_format.fmt.pix.sizeimage;
  else
    size = Frame::storageSize( m_typecode,
                               m_format.fmt.pix.width,
                               m_format.fmt.pix.height );
  if ( m_io == IO_READ ) {
    FramePtr image( new Frame( m_typecode,
                               m_format.fmt.pix.width,
                               m_format.fmt.pix.height ) );
    ERRORMACRO( ::read( m_fd, image->data(), size ) != -1,
                Error, , "Error reading from device \"" << m_device << "\": "
                << strerror( errno ) );
    retVal = image;
  } else {
    assert( m_io == IO_MMAP || m_io == IO_USERPTR );
    if ( m_frameUsed ) {
      ERRORMACRO( xioctl( VIDIOC_QBUF, &m_frame ) == 0, Error, ,
                  "Error enqueuing buffer for device \""
                  << m_device << '"' );
      m_frameUsed = false;
    };
    memset( &m_frame, 0, sizeof(m_frame) );
    m_frame.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_frame.memory = m_io == IO_MMAP ? V4L2_MEMORY_MMAP :
                                       V4L2_MEMORY_USERPTR;
    ERRORMACRO( xioctl( VIDIOC_DQBUF, &m_frame ) == 0, Error, ,
                "Error reading from device \"" << m_device << "\": "
                << strerror( errno ) );
    char *p =
      (char *)( m_io == IO_MMAP ? m_map[ m_frame.index ] :
                         m_user[ m_frame.index ] );
    m_frameUsed = true;
    retVal = FramePtr
      ( new Frame( m_typecode,
                   m_format.fmt.pix.width, m_format.fmt.pix.height, p ) );
  };
  return retVal;
}

VALUE V4L2Input::registerRubyClass( VALUE module )
{
  cRubyClass = rb_define_class_under( module, "V4L2Input", rb_cObject );
  rb_define_const( cRubyClass, "MODE_RGB332",
                   INT2NUM(V4L2_PIX_FMT_RGB332) );
  rb_define_const( cRubyClass, "MODE_RGB444",
                   INT2NUM(V4L2_PIX_FMT_RGB444) );
  rb_define_const( cRubyClass, "MODE_RGB555",
                   INT2NUM(V4L2_PIX_FMT_RGB555) );
  rb_define_const( cRubyClass, "MODE_RGB565",
                   INT2NUM(V4L2_PIX_FMT_RGB565) );
  rb_define_const( cRubyClass, "MODE_RGB555X",
                   INT2NUM(V4L2_PIX_FMT_RGB555X) );
  rb_define_const( cRubyClass, "MODE_RGB565X",
                   INT2NUM(V4L2_PIX_FMT_RGB565X) );
  rb_define_const( cRubyClass, "MODE_BGR24",
                   INT2NUM(V4L2_PIX_FMT_BGR24) );
  rb_define_const( cRubyClass, "MODE_RGB24",
                   INT2NUM(V4L2_PIX_FMT_RGB24) );
  rb_define_const( cRubyClass, "MODE_BGR32",
                   INT2NUM(V4L2_PIX_FMT_BGR32) );
  rb_define_const( cRubyClass, "MODE_RGB32",
                   INT2NUM(V4L2_PIX_FMT_RGB32) );
  rb_define_const( cRubyClass, "MODE_GREY",
                   INT2NUM(V4L2_PIX_FMT_GREY) );
  rb_define_const( cRubyClass, "MODE_Y16",
                   INT2NUM(V4L2_PIX_FMT_Y16) );
  rb_define_const( cRubyClass, "MODE_PAL8",
                   INT2NUM(V4L2_PIX_FMT_PAL8) );
  rb_define_const( cRubyClass, "MODE_YVU410",
                   INT2NUM(V4L2_PIX_FMT_YVU410) );
  rb_define_const( cRubyClass, "MODE_YVU420",
                   INT2NUM(V4L2_PIX_FMT_YVU420) );
  rb_define_const( cRubyClass, "MODE_YUYV",
                   INT2NUM(V4L2_PIX_FMT_YUYV) );
  rb_define_const( cRubyClass, "MODE_YYUV",
                   INT2NUM(V4L2_PIX_FMT_YYUV) );
  rb_define_const( cRubyClass, "MODE_YVYU",
                   INT2NUM(V4L2_PIX_FMT_YVYU) );
  rb_define_const( cRubyClass, "MODE_UYVY",
                   INT2NUM(V4L2_PIX_FMT_UYVY) );
#ifdef V4L2_PIX_FMT_VYUY
  rb_define_const( cRubyClass, "MODE_VYUY",
                   INT2NUM(V4L2_PIX_FMT_VYUY) );
#endif
  rb_define_const( cRubyClass, "MODE_YUV422P",
                   INT2NUM(V4L2_PIX_FMT_YUV422P) );
  rb_define_const( cRubyClass, "MODE_YUV411P",
                   INT2NUM(V4L2_PIX_FMT_YUV411P) );
  rb_define_const( cRubyClass, "MODE_Y41P",
                   INT2NUM(V4L2_PIX_FMT_Y41P) );
  rb_define_const( cRubyClass, "MODE_YUV444",
                   INT2NUM(V4L2_PIX_FMT_YUV444) );
  rb_define_const( cRubyClass, "MODE_YUV555",
                   INT2NUM(V4L2_PIX_FMT_YUV555) );
  rb_define_const( cRubyClass, "MODE_YUV565",
                   INT2NUM(V4L2_PIX_FMT_YUV565) );
  rb_define_const( cRubyClass, "MODE_YUV32",
                   INT2NUM(V4L2_PIX_FMT_YUV32) );
  rb_define_const( cRubyClass, "MODE_YUV410",
                   INT2NUM(V4L2_PIX_FMT_YUV410) );
  rb_define_const( cRubyClass, "MODE_YUV420",
                   INT2NUM(V4L2_PIX_FMT_YUV420) );
  rb_define_const( cRubyClass, "MODE_HI240",
                   INT2NUM(V4L2_PIX_FMT_HI240) );
  rb_define_const( cRubyClass, "MODE_HM12",
                   INT2NUM(V4L2_PIX_FMT_HM12) );
  rb_define_const( cRubyClass, "MODE_NV12",
                   INT2NUM(V4L2_PIX_FMT_NV12) );
  rb_define_const( cRubyClass, "MODE_NV21",
                   INT2NUM(V4L2_PIX_FMT_NV21) );
#ifdef V4L2_PIX_FMT_NV16
  rb_define_const( cRubyClass, "MODE_NV16",
                   INT2NUM(V4L2_PIX_FMT_NV16) );
#endif
#ifdef V4L2_PIX_FMT_NV61
  rb_define_const( cRubyClass, "MODE_NV61",
                   INT2NUM(V4L2_PIX_FMT_NV61) );
#endif
  rb_define_const( cRubyClass, "MODE_SBGGR8",
                   INT2NUM(V4L2_PIX_FMT_SBGGR8) );
  rb_define_const( cRubyClass, "MODE_SGBRG8",
                   INT2NUM(V4L2_PIX_FMT_SGBRG8) );
#ifdef V4L2_PIX_FMT_SGRBG8
  rb_define_const( cRubyClass, "MODE_SGRBG8",
                   INT2NUM(V4L2_PIX_FMT_SGRBG8) );
#endif
  rb_define_const( cRubyClass, "MODE_SGRBG10",
                   INT2NUM(V4L2_PIX_FMT_SGRBG10) );
  rb_define_const( cRubyClass, "MODE_SGRBG10DPCM8",
                   INT2NUM(V4L2_PIX_FMT_SGRBG10DPCM8) );
  rb_define_const( cRubyClass, "MODE_SBGGR16",
                   INT2NUM(V4L2_PIX_FMT_SBGGR16) );
  rb_define_const( cRubyClass, "MODE_MJPEG",
                   INT2NUM(V4L2_PIX_FMT_MJPEG) );
  rb_define_const( cRubyClass, "MODE_JPEG",
                   INT2NUM(V4L2_PIX_FMT_JPEG) );
  rb_define_const( cRubyClass, "MODE_DV",
                   INT2NUM(V4L2_PIX_FMT_DV) );
  rb_define_const( cRubyClass, "MODE_MPEG",
                   INT2NUM(V4L2_PIX_FMT_MPEG) );
  rb_define_const( cRubyClass, "MODE_WNVA",
                   INT2NUM(V4L2_PIX_FMT_WNVA) );
  rb_define_const( cRubyClass, "MODE_SN9C10X",
                   INT2NUM(V4L2_PIX_FMT_SN9C10X) );
#ifdef V4L2_PIX_FMT_SN9C20X_I420
  rb_define_const( cRubyClass, "MODE_SN9C20X_I420",
                   INT2NUM(V4L2_PIX_FMT_SN9C20X_I420) );
#endif
  rb_define_const( cRubyClass, "MODE_PWC1",
                   INT2NUM(V4L2_PIX_FMT_PWC1) );
  rb_define_const( cRubyClass, "MODE_PWC2",
                   INT2NUM(V4L2_PIX_FMT_PWC2) );
  rb_define_const( cRubyClass, "MODE_ET61X251",
                   INT2NUM(V4L2_PIX_FMT_ET61X251) );
  rb_define_const( cRubyClass, "MODE_SPCA501",
                   INT2NUM(V4L2_PIX_FMT_SPCA501) );
  rb_define_const( cRubyClass, "MODE_SPCA505",
                   INT2NUM(V4L2_PIX_FMT_SPCA505) );
  rb_define_const( cRubyClass, "MODE_SPCA508",
                   INT2NUM(V4L2_PIX_FMT_SPCA508) );
  rb_define_const( cRubyClass, "MODE_SPCA561",
                   INT2NUM(V4L2_PIX_FMT_SPCA561) );
  rb_define_const( cRubyClass, "MODE_PAC207",
                   INT2NUM(V4L2_PIX_FMT_PAC207) );
#ifdef V4L2_PIX_FMT_MR97310A
  rb_define_const( cRubyClass, "MODE_MR97310A",
                   INT2NUM(V4L2_PIX_FMT_MR97310A) );
#endif
#ifdef V4L2_PIX_FMT_SQ905C
  rb_define_const( cRubyClass, "MODE_SQ905C",
                   INT2NUM(V4L2_PIX_FMT_SQ905C) );
#endif
  rb_define_const( cRubyClass, "MODE_PJPG",
                   INT2NUM(V4L2_PIX_FMT_PJPG) );
#ifdef V4L2_PIX_FMT_OV511
  rb_define_const( cRubyClass, "MODE_OV511",
                   INT2NUM(V4L2_PIX_FMT_OV511) );
#endif
#ifdef V4L2_PIX_FMT_OV518
  rb_define_const( cRubyClass, "MODE_OV518",
                   INT2NUM(V4L2_PIX_FMT_OV518) );
#endif
  rb_define_const( cRubyClass, "TYPE_INTEGER",
                   INT2NUM(V4L2_CTRL_TYPE_INTEGER) );
  rb_define_const( cRubyClass, "TYPE_BOOLEAN",
                   INT2NUM(V4L2_CTRL_TYPE_BOOLEAN) );
  rb_define_const( cRubyClass, "TYPE_MENU",
                   INT2NUM(V4L2_CTRL_TYPE_MENU) );
  rb_define_const( cRubyClass, "TYPE_BUTTON",
                   INT2NUM(V4L2_CTRL_TYPE_BUTTON) );
  rb_define_const( cRubyClass, "TYPE_CTRL_CLASS",
                   INT2NUM(V4L2_CTRL_TYPE_CTRL_CLASS) );
  rb_define_const( cRubyClass, "FEATURE_BASE",
                   INT2NUM(V4L2_CID_BASE) );
  rb_define_const( cRubyClass, "FEATURE_USER_BASE",
                   INT2NUM(V4L2_CID_USER_BASE) );
  rb_define_const( cRubyClass, "FEATURE_PRIVATE_BASE",
                   INT2NUM(V4L2_CID_PRIVATE_BASE) );
  rb_define_const( cRubyClass, "FEATURE_BRIGHTNESS",
                   INT2NUM(V4L2_CID_BRIGHTNESS) );
  rb_define_const( cRubyClass, "FEATURE_CONTRAST",
                   INT2NUM(V4L2_CID_CONTRAST) );
  rb_define_const( cRubyClass, "FEATURE_SATURATION",
                   INT2NUM(V4L2_CID_SATURATION) );
  rb_define_const( cRubyClass, "FEATURE_HUE",
                   INT2NUM(V4L2_CID_HUE) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_VOLUME",
                   INT2NUM(V4L2_CID_AUDIO_VOLUME) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_BALANCE",
                   INT2NUM(V4L2_CID_AUDIO_BALANCE) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_BASS",
                   INT2NUM(V4L2_CID_AUDIO_BASS) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_TREBLE",
                   INT2NUM(V4L2_CID_AUDIO_TREBLE) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_MUTE",
                   INT2NUM(V4L2_CID_AUDIO_MUTE) );
  rb_define_const( cRubyClass, "FEATURE_AUDIO_LOUDNESS",
                   INT2NUM(V4L2_CID_AUDIO_LOUDNESS) );
  rb_define_const( cRubyClass, "FEATURE_BLACK_LEVEL",
                   INT2NUM(V4L2_CID_BLACK_LEVEL) );
  rb_define_const( cRubyClass, "FEATURE_AUTO_WHITE_BALANCE",
                   INT2NUM(V4L2_CID_AUTO_WHITE_BALANCE) );
  rb_define_const( cRubyClass, "FEATURE_DO_WHITE_BALANCE",
                   INT2NUM(V4L2_CID_DO_WHITE_BALANCE) );
  rb_define_const( cRubyClass, "FEATURE_RED_BALANCE",
                   INT2NUM(V4L2_CID_RED_BALANCE) );
  rb_define_const( cRubyClass, "FEATURE_BLUE_BALANCE",
                   INT2NUM(V4L2_CID_BLUE_BALANCE) );
  rb_define_const( cRubyClass, "FEATURE_GAMMA",
                   INT2NUM(V4L2_CID_GAMMA) );
  rb_define_const( cRubyClass, "FEATURE_WHITENESS",
                   INT2NUM(V4L2_CID_WHITENESS) );
  rb_define_const( cRubyClass, "FEATURE_EXPOSURE",
                   INT2NUM(V4L2_CID_EXPOSURE) );
  rb_define_const( cRubyClass, "FEATURE_AUTOGAIN",
                   INT2NUM(V4L2_CID_AUTOGAIN) );
  rb_define_const( cRubyClass, "FEATURE_GAIN",
                   INT2NUM(V4L2_CID_GAIN) );
  rb_define_const( cRubyClass, "FEATURE_HFLIP",
                   INT2NUM(V4L2_CID_HFLIP) );
  rb_define_const( cRubyClass, "FEATURE_VFLIP",
                   INT2NUM(V4L2_CID_VFLIP) );
  rb_define_const( cRubyClass, "FEATURE_HCENTER",
                   INT2NUM(V4L2_CID_HCENTER) );
  rb_define_const( cRubyClass, "FEATURE_VCENTER",
                   INT2NUM(V4L2_CID_VCENTER) );
  rb_define_const( cRubyClass, "FEATURE_LASTP1",
                   INT2NUM(V4L2_CID_LASTP1) );
  rb_define_singleton_method( cRubyClass, "new", RUBY_METHOD_FUNC( wrapNew ), 1 );
  rb_define_method( cRubyClass, "close",
                    RUBY_METHOD_FUNC( wrapClose ), 0 );
  rb_define_method( cRubyClass, "read",
                    RUBY_METHOD_FUNC( wrapRead ), 0 );
  rb_define_method( cRubyClass, "status?",
                    RUBY_METHOD_FUNC( wrapStatus ), 0 );
  rb_define_method( cRubyClass, "width",
                    RUBY_METHOD_FUNC( wrapWidth ), 0 );
  rb_define_method( cRubyClass, "height",
                    RUBY_METHOD_FUNC( wrapHeight ), 0 );
  rb_define_method( cRubyClass, "feature_exist?",
                    RUBY_METHOD_FUNC( wrapHasFeature ), 1 );
  rb_define_method( cRubyClass, "feature_read",
                    RUBY_METHOD_FUNC( wrapFeatureValue ), 1 );
  rb_define_method( cRubyClass, "feature_write",
                    RUBY_METHOD_FUNC( wrapSetFeatureValue ), 2 );
  rb_define_method( cRubyClass, "feature_type",
                    RUBY_METHOD_FUNC( wrapFeatureType ), 1 );
  rb_define_method( cRubyClass, "feature_name",
                    RUBY_METHOD_FUNC( wrapFeatureName ), 1 );
  rb_define_method( cRubyClass, "feature_min",
                    RUBY_METHOD_FUNC( wrapFeatureMin ), 1 );
  rb_define_method( cRubyClass, "feature_max",
                    RUBY_METHOD_FUNC( wrapFeatureMax ), 1 );
  rb_define_method( cRubyClass, "feature_step",
                    RUBY_METHOD_FUNC( wrapFeatureStep ), 1 );
  rb_define_method( cRubyClass, "feature_default_value",
                    RUBY_METHOD_FUNC( wrapFeatureDefaultValue ), 1 );
  return cRubyClass;
}

void V4L2Input::deleteRubyObject( void *ptr )
{
  delete (V4L2InputPtr *)ptr;
}

VALUE V4L2Input::wrapNew( VALUE rbClass, VALUE rbDevice )
{
  VALUE retVal = Qnil;
  try {
    rb_check_type( rbDevice, T_STRING );
    V4L2SelectPtr select( new V4L2Select );
    V4L2InputPtr ptr( new V4L2Input( StringValuePtr( rbDevice ), select ) );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new V4L2InputPtr( ptr ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE V4L2Input::wrapClose( VALUE rbSelf )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  (*self)->close();
  return rbSelf;
}

VALUE V4L2Input::wrapRead( VALUE rbSelf )
{
  VALUE retVal = Qnil;
  try {
    V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
    FramePtr frame( (*self)->read() );
    retVal = frame->rubyObject();
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE V4L2Input::wrapStatus( VALUE rbSelf )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  return (*self)->status() ? Qtrue : Qfalse;
}

VALUE V4L2Input::wrapWidth( VALUE rbSelf )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  return INT2NUM((*self)->width());
}

VALUE V4L2Input::wrapHeight( VALUE rbSelf )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  return INT2NUM((*self)->height());
}

VALUE V4L2Input::wrapHasFeature( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = (*self)->hasFeature( NUM2INT( rbId ) ) ? Qtrue : Qfalse;
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureValue( VALUE rbSelf, VALUE rbId )
{
  VALUE retVal = Qnil;
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  try {
    retVal = INT2NUM((*self)->featureValue( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE V4L2Input::wrapSetFeatureValue( VALUE rbSelf, VALUE rbId,
                                      VALUE rbValue )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  try {
    (*self)->setFeatureValue( NUM2INT( rbId ), NUM2INT( rbValue ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return Qnil;
};

VALUE V4L2Input::wrapFeatureType( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = INT2NUM( (int)(*self)->featureType( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureName( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = rb_str_new2( (*self)->featureName( NUM2INT( rbId ) ).c_str() );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureMin( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = INT2NUM( (*self)->featureMin( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureMax( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = INT2NUM( (*self)->featureMax( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureStep( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = INT2NUM( (*self)->featureStep( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

VALUE V4L2Input::wrapFeatureDefaultValue( VALUE rbSelf, VALUE rbId )
{
  V4L2InputPtr *self; Data_Get_Struct( rbSelf, V4L2InputPtr, self );
  VALUE retVal = Qnil;
  try {
    retVal = INT2NUM( (*self)->featureDefaultValue( NUM2INT( rbId ) ) );
  } catch ( std::exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
};

int V4L2Input::xioctl( int request, void *arg )
{
  int r;
  do {
    r = ioctl( m_fd, request, arg );
#ifndef NDEBUG
    if ( r == -1 && errno == EINTR )
      cerr << "ioctl returned " << r << endl
           << "errno is " << strerror( errno ) << endl;
#endif
  } while ( r == -1 && errno == EINTR );
  return r;
}

