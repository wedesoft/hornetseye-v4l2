hornetseye-v4l2
===============
This Ruby extension provides camera input using Video for Linux version 2.

**Author**:       Jan Wedekind
**Copyright**:    2010
**License**:      GPL

Synopsis
--------

This Ruby extension provides the class {Hornetseye::V4L2Input} for capturing video frames using Video for Linux version 2 (V4L2).

Installation
------------

*hornetseye-v4l2* requires the V4L2 headers. If you are running Debian or (K)ubuntu, you can install them like this:

    $ sudo aptitude install linux-libc-dev libswscale-dev libboost-dev

To install this Ruby extension, use the following command:

    $ sudo gem install hornetseye-v4l2

Alternatively you can build and install the Ruby extension from source as follows:

    $ rake
    $ sudo rake install

Usage
-----

Simply run Interactive Ruby:

    $ irb

You can open a V4L2-compatible camera as shown below. This example will open the camera and switch to a resolution selected by the user. Finally the camera input is displayed in a window. This example requires *hornetseye-xorg* in addition to this Ruby extension.

    require 'rubygems'
    require 'hornetseye_v4l2'
    require 'hornetseye_xorg'
    include Hornetseye
    camera = V4L2Input.new '/dev/video0' do |modes|
      modes.each_with_index { |mode,i| puts "#{i + 1}: #{mode}" }
      modes[ STDIN.readline.to_i - 1 ]
    end
    X11Display.show { camera.read }

