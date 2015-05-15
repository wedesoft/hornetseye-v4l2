#!/usr/bin/env ruby
require 'date'
require 'rake/clean'
require 'rake/testtask'
require 'rake/packagetask'
require 'rake/loaders/makefile'
require 'rbconfig'
require_relative 'config'

OBJ = CC_FILES.ext 'o'
$CXXFLAGS = "-DNDEBUG #{CFG[ 'CPPFLAGS' ]} #{CFG[ 'CFLAGS' ]}"
if CFG['rubyarchhdrdir']
  $CXXFLAGS = "#{$CXXFLAGS} -I#{CFG['rubyhdrdir']} -I#{CFG['rubyarchhdrdir']}"
elsif CFG['rubyhdrdir']
  $CXXFLAGS = "#{$CXXFLAGS} -I#{CFG['rubyhdrdir' ]} -I#{CFG['rubyhdrdir']}/#{CFG['arch']}"
else
  $CXXFLAGS = "#{$CXXFLAGS} -I#{CFG[ 'archdir' ]}"
end
$LIBRUBYARG = "-L#{CFG[ 'libdir' ]} #{CFG[ 'LIBRUBYARG' ]} #{CFG[ 'LDFLAGS' ]} " +
              "#{CFG[ 'SOLIBS' ]} #{CFG[ 'DLDLIBS' ]}"
$SITELIBDIR = CFG[ 'sitelibdir' ]
$SITEARCHDIR = CFG[ 'sitearchdir' ]
$LDSHARED = CFG[ 'LDSHARED' ][ CFG[ 'LDSHARED' ].index( ' ' ) .. -1 ]

task :default => :all

desc 'Compile Ruby extension (default)'
task :all => [ SO_FILE ]

file SO_FILE => OBJ do |t|
   sh "#{CXX} -shared -o #{t.name} #{OBJ} #{$LIBRUBYARG}"
end

task :test => [ SO_FILE ]

desc 'Install Ruby extension'
task :install => :all do
  verbose true do
    for f in RB_FILES do
      FileUtils.mkdir_p "#{$SITELIBDIR}/#{File.dirname f.gsub( /^lib\//, '' )}"
      FileUtils.cp_r f, "#{$SITELIBDIR}/#{f.gsub( /^lib\//, '' )}"
    end
    FileUtils.mkdir_p $SITEARCHDIR
    FileUtils.cp SO_FILE, "#{$SITEARCHDIR}/#{File.basename SO_FILE}"
  end
end

desc 'Uninstall Ruby extension'
task :uninstall do
  verbose true do
    for f in RB_FILES do
      FileUtils.rm_f "#{$SITELIBDIR}/#{f.gsub /^lib\//, ''}"
    end
    FileUtils.rm_f "#{$SITEARCHDIR}/#{File.basename SO_FILE}"
  end
end

Rake::TestTask.new do |t|
  t.libs << 'ext'
  t.test_files = TC_FILES
end

begin
  require 'yard'
  YARD::Rake::YardocTask.new :yard do |y|
    y.files << RB_FILES
  end
rescue LoadError
  STDERR.puts 'Please install \'yard\' if you want to generate documentation'
end

Rake::PackageTask.new PKG_NAME, PKG_VERSION do |p|
  p.need_tar = true
  p.package_files = PKG_FILES
end

rule '.o' => '.cc' do |t|
   sh "#{CXX} #{$CXXFLAGS} -c -o #{t.name} #{t.source}"
end

file ".depends.mf" do |t|
  sh "g++ -MM #{CC_FILES.join ' '} | " +
    "sed -e :a -e N -e 's/\\n/\\$/g' -e ta | " +
    "sed -e 's/ *\\\\\\$ */ /g' -e 's/\\$/\\n/g' | sed -e 's/^/ext\\//' > #{t.name}"
end
CC_FILES.each do |t|
  file t.ext(".o") => t
end
import ".depends.mf"

CLEAN.include 'ext/*.o'
CLOBBER.include SO_FILE, 'doc', '.yardoc', '.depends.mf'

