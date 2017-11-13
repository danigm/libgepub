project(
  'libgepub', 'c',
  version: '0.5.2',
  license: 'LGPL2+',
  default_options: 'buildtype=debugoptimized',
  meson_version: '>= 0.41.0'
)

gepub_version = meson.project_version()
version_array = gepub_version.split('.')
gepub_major_version = version_array[0].to_int()
gepub_minor_version = version_array[1].to_int()
gepub_version_micro = version_array[2].to_int()
gepub_api_version = '@0@.@1@'.format(gepub_major_version, gepub_minor_version)

gepub_gir_ns = 'Gepub'
gepub_gir_version = '1.0'

gepub_prefix = get_option('prefix')
gepub_datadir = join_paths(gepub_prefix, get_option('datadir'))
gepub_libdir = join_paths(gepub_prefix, get_option('libdir'))
gepub_libexecdir = join_paths(gepub_prefix, get_option('libexecdir'))
gepub_includedir = join_paths(gepub_prefix, get_option('includedir'))

soversion = 0
current = 0
revision = 0
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

cc = meson.get_compiler('c')

gepub_deps = [
  dependency('webkit2gtk-4.0'),
  dependency('libsoup-2.4'),
  dependency('glib-2.0'),
  dependency('gobject-2.0'),
  dependency('gio-2.0'),
  dependency('libxml-2.0'),
  dependency('libarchive')
]

gnome = import('gnome')
pkg = import('pkgconfig')

top_inc = include_directories('.')

subdir('libgepub')
subdir('tests')

configure_file(
  output: 'config.h',
  configuration: configuration_data()
)
