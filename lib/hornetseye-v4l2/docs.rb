# hornetseye-xorg - Graphical output under X.Org
# Copyright (C) 2010 Jan Wedekind
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Namespace of Hornetseye computer vision library
module Hornetseye

  # Class for handling a V4L2-compatible camera
  #
  # @see http://v4l2spec.bytesex.org/spec-single/v4l2.html
  class V4L2Input

    class << self

      # Video mode
      #
      # @private
      MODE_RGB332 = nil

      # Video mode
      #
      # @private
      MODE_RGB444 = nil

      # Video mode
      #
      # @private
      MODE_RGB555 = nil

      # Video mode
      #
      # @private
      MODE_RGB565 = nil

      # Video mode
      #
      # @private
      MODE_RGB555X = nil

      # Video mode
      #
      # @private
      MODE_RGB565X = nil

      # Video mode
      #
      # @private
      MODE_BGR24 = nil

      # Video mode
      #
      # @private
      MODE_RGB24 = nil

      # Video mode
      #
      # @private
      MODE_BGR32 = nil

      # Video mode
      #
      # @private
      MODE_RGB32 = nil

      # Video mode
      #
      # @private
      MODE_GREY = nil

      # Video mode
      #
      # @private
      MODE_Y16 = nil

      # Video mode
      #
      # @private
      MODE_PAL8 = nil

      # Video mode
      #
      # @private
      MODE_YVU410 = nil

      # Video mode
      #
      # @private
      MODE_YVU420 = nil

      # Video mode
      #
      # @private
      MODE_YUYV = nil

      # Video mode
      #
      # @private
      MODE_YYUV = nil

      # Video mode
      #
      # @private
      MODE_YVYU = nil

      # Video mode
      #
      # @private
      MODE_UYVY = nil

      # Video mode
      #
      # @private
      MODE_VYUY = nil

      # Video mode
      #
      # @private
      MODE_YUV422P = nil

      # Video mode
      #
      # @private
      MODE_YUV411P = nil

      # Video mode
      #
      # @private
      MODE_Y41P = nil

      # Video mode
      #
      # @private
      MODE_YUV444 = nil

      # Video mode
      #
      # @private
      MODE_YUV555 = nil

      # Video mode
      #
      # @private
      MODE_YUV565 = nil

      # Video mode
      #
      # @private
      MODE_YUV32 = nil

      # Video mode
      #
      # @private
      MODE_YUV410 = nil

      # Video mode
      #
      # @private
      MODE_YUV420 = nil

      # Video mode
      #
      # @private
      MODE_HI240 = nil

      # Video mode
      #
      # @private
      MODE_HM12 = nil

      # Video mode
      #
      # @private
      MODE_NV12 = nil

      # Video mode
      #
      # @private
      MODE_NV21 = nil

      # Video mode
      #
      # @private
      MODE_NV16 = nil

      # Video mode
      #
      # @private
      MODE_NV61 = nil

      # Video mode
      #
      # @private
      MODE_SBGGR8 = nil

      # Video mode
      #
      # @private
      MODE_SGBRG8 = nil

      # Video mode
      #
      # @private
      MODE_SGRBG8 = nil

      # Video mode
      #
      # @private
      MODE_SGRBG10 = nil

      # Video mode
      #
      # @private
      MODE_SGRBG10DPCM8 = nil

      # Video mode
      #
      # @private
      MODE_SBGGR16 = nil

      # Video mode
      #
      # @private
      MODE_MJPEG = nil

      # Video mode
      #
      # @private
      MODE_JPEG = nil

      # Video mode
      #
      # @private
      MODE_DV = nil

      # Video mode
      #
      # @private
      MODE_MPEG = nil

      # Video mode
      #
      # @private
      MODE_WNVA = nil

      # Video mode
      #
      # @private
      MODE_SN9C10X = nil

      # Video mode
      #
      # @private
      MODE_SN9C20X_I420 = nil

      # Video mode
      #
      # @private
      MODE_PWC1 = nil

      # Video mode
      #
      # @private
      MODE_PWC2 = nil

      # Video mode
      #
      # @private
      MODE_ET61X251 = nil

      # Video mode
      #
      # @private
      MODE_SPCA501 = nil

      # Video mode
      #
      # @private
      MODE_SPCA505 = nil

      # Video mode
      #
      # @private
      MODE_SPCA508 = nil

      # Video mode
      #
      # @private
      MODE_SPCA561 = nil

      # Video mode
      #
      # @private
      MODE_PAC207 = nil

      # Video mode
      #
      # @private
      MODE_MR97310A = nil

      # Video mode
      #
      # @private
      MODE_SQ905C = nil

      # Video mode
      #
      # @private
      MODE_PJPG = nil

      # Video mode
      #
      # @private
      MODE_OV511 = nil

      # Video mode
      #
      # @private
      MODE_OV518 = nil

      # Feature type
      TYPE_INTEGER = nil

      # Feature type
      TYPE_BOOLEAN = nil

      # Feature type
      TYPE_MENU = nil

      # Feature type
      TYPE_BUTTON = nil

      # Feature type
      TYPE_CTRL_CLASS = nil

      # First feature
      FEATURE_BASE = nil

      # First custom feature
      FEATURE_USER_BASE = nil

      # First standard feature
      FEATURE_PRIVATE_BASE = nil

      # A feature
      FEATURE_BRIGHTNESS = nil

      # A feature
      FEATURE_CONTRAST = nil

      # A feature
      FEATURE_SATURATION = nil

      # A feature
      FEATURE_HUE = nil

      # A feature
      FEATURE_AUDIO_VOLUME = nil

      # A feature
      FEATURE_AUDIO_BALANCE = nil

      # A feature
      FEATURE_AUDIO_BASS = nil

      # A feature
      FEATURE_AUDIO_TREBLE = nil

      # A feature
      FEATURE_AUDIO_MUTE = nil

      # A feature
      FEATURE_AUDIO_LOUDNESS = nil

      # A feature
      FEATURE_BLACK_LEVEL = nil

      # A feature
      FEATURE_AUTO_WHITE_BALANCE = nil

      # A feature
      FEATURE_DO_WHITE_BALANCE = nil

      # A feature
      FEATURE_RED_BALANCE = nil

      # A feature
      FEATURE_BLUE_BALANCE = nil

      # A feature
      FEATURE_GAMMA = nil

      # A feature
      FEATURE_WHITENESS = nil

      # A feature
      FEATURE_EXPOSURE = nil

      # A feature
      FEATURE_AUTOGAIN = nil

      # A feature
      FEATURE_GAIN = nil

      # A feature
      FEATURE_HFLIP = nil

      # A feature
      FEATURE_VFLIP = nil

      # A feature
      FEATURE_HCENTER = nil

      # A feature
      FEATURE_VCENTER = nil

      # A feature
      FEATURE_LASTP1 = nil

    end

    # Close the video device
    #
    # @return [V4L2Input] Returns +self+.
    def close
    end

    # Read a video frame
    #
    # @return [MultiArray,Frame_] The video frame.
    def read
    end

    # Check whether device is not closed
    #
    # @return [Boolean] Returns +true+ as long as device is open.
    def status?
    end

    # Width of video input
    #
    # @return [Integer] Width of video frames.
    def width
    end

    # Height of video input
    #
    # @return [Integer] Width of video frames.
    def height
    end

    # Check for existence of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Boolean] Returns +true+ if this feature is supported.
    def feature_exist?( id )
    end

    # Get value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Current value of feature.
    def feature_read( id )
    end

    # Set value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Returns +value+.
    def feature_write( id, value )
    end

    # Get type of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Type of feature.
    def feature_type( id )
    end

    # Get name of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [String] Name of feature.
    def feature_name( id )
    end

    # Get minimum value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Minimum value.
    def feature_min( id )
    end

    # Get maximum value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Maximum value.
    def feature_max( id )
    end

    # Get step size of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Step size.
    def feature_step( id )
    end

    # Get default value of feature
    #
    # @param [Integer] id Feature identifier.
    #
    # @return [Integer] Default value.
    def feature_default_value( id )
    end
 
  end

end

