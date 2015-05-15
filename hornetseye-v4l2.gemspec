require_relative 'config'

Gem::Specification.new do |s|
  s.name = PKG_NAME
  s.version = PKG_VERSION
  s.platform = Gem::Platform::RUBY
  s.date = Date.today.to_s
  s.summary = SUMMARY
  s.description = DESCRIPTION
  s.license = LICENSE
  s.author = AUTHOR
  s.email = EMAIL
  s.homepage = HOMEPAGE
  s.files = PKG_FILES
  s.test_files = TC_FILES
  s.require_paths = [ 'lib', 'ext' ]
  s.rubyforge_project = %q{hornetseye}
  s.extensions = %w{Rakefile}
  s.has_rdoc = 'yard'
  s.extra_rdoc_files = []
  s.rdoc_options = %w{--no-private}
  s.add_dependency %<malloc>, [ '~> 1.1' ]
  s.add_dependency %<multiarray>, [ '~> 1.0' ]
  s.add_dependency %<hornetseye-frame>, [ '~> 1.0' ]
end
