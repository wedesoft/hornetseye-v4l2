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

      alias_method :orig_new, :new

      # Open a camera device for input
      #
      # The device is opened and a list of supported resolutions is handed back as
      # parameter to the code block. The code block must return the selected mode
      # so that initialisation can be completed. If no block is given, the highest
      # available resolution is selected after giving colour modes preference.
      #
      # @example Open a camera device
      #   require 'hornetseye_v4l2'
      #   include Hornetseye
      #   camera = V4L2Input.new { |modes| modes.last }
      #
      # @param [String] device The device file name.
      # @param [Integer] channel Number of the input channel to use.
      # @param [Proc] action An optional block for selecting the desired video mode.
      # @return [V4L2Input] An object for accessing the camera.
      def new( device = '/dev/video0', channel = 0, &action )
        orig_new device, channel do |modes|
          map = { MODE_UYVY   => UYVY,
                  MODE_YUYV   => YUY2,
                  MODE_YUV420 => I420,
                  MODE_GREY   => UBYTE,
                  MODE_RGB24  => UBYTERGB,
                  MODE_BGR24  => BGR }
          modes.each do |mode|
            unless map[mode.first]
              warn "Unsupported video mode #{"0x%08x" % mode.first} #{mode[1]}x#{mode[2]}"
            end
          end
          frame_types = modes.collect { |mode| [map[mode.first], *mode[1 .. 2]] }.
            select { |mode| mode.first }
          if action
            desired = action.call frame_types
          else
            preference = [I420, UYVY, YUY2, UBYTERGB, BGR, UBYTE]
            desired = frame_types.sort_by do |mode|
              [-preference.index(mode.first), mode[1] * mode[2]]
            end.last
            raise "Device does not support a known video mode" unless desired
          end
          mode = map.invert[desired[0]]
          raise "Video mode #{desired.typecode} not supported" unless mode
          [mode, desired[1], desired[2]]
        end
      end

    end

    include ReaderConversion

  end

end

