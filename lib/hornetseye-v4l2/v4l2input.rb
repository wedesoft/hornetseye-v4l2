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

  class V4L2Input

    class << self

      alias_method :orig_new, :new

      def new( device = '/dev/video0', &action )
        orig_new device do |modes|
          map = { MODE_UYVY   => UYVY,
                  MODE_YUYV   => YUY2,
                  MODE_YUV420 => I420,
                  MODE_GREY   => UBYTE,
                  MODE_RGB24  => UBYTERGB }
          frame_types, index = [], []
          modes.each_with_index do |mode,i|
            target = map[ mode.first ]
            if target
              frame_types.push Hornetseye::Frame( target, *mode[ 1 .. 2 ] )
              index.push i
            end
          end
          desired = action.call frame_types
          unless frame_types.member? desired
            raise "Frame type #{desired} not supported by camera"
          end
          index[ frame_types.index( desired ) ]
        end
      end

    end

  end

end

