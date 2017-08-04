project(
  'libgepub', 'c',
  version: '0.5',
  license: 'LGPL2+',
  default_options: [
    'buildtype=debugoptimized',
    'warning_level=1'
  ],
  meson_version: '>= 0.41.0'
)

gepub_version = meson.project_version()
version_array = gepub_version.split('.')
gepub_major_version = version_array[0].to_int()
gepub_minor_version = version_array[1].to_int()

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

config_h = configuration_data()

# defines
set_defines = [
  # package
  ['PACKAGE', meson.project_name()],
  ['PACKAGE_API_VERSION', gepub_version],
  ['PACKAGE_BUGREPORT', 'danigm@wadobo.com'],
  ['PACKAGE_NAME', meson.project_name()],
  ['PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), gepub_version)],
  ['PACKAGE_TARNAME', meson.project_name()],
  ['PACKAGE_URL', ''],
  ['PACKAGE_VERSION', gepub_version],
  ['VERSION', gepub_version]
]

foreach define: set_defines
  config_h.set_quoted(define[0], define[1])
endforeach

# headers
check_headers = [
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_INTTYPES_H', 'inttypes.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_UNISTD_H', 'unistd.h']
]

foreach header: check_headers
  config_h.set(header[0], cc.has_header(header[1]))
endforeach

# compiler flags
common_flags = ['-DHAVE_CONFIG_H']

add_project_arguments(common_flags, language: 'c')

gepub_deps = [
  dependency('webkit2gtk-4.0'),
  dependency('libsoup-2.4'),
  dependency('glib-2.0'),
  dependency('gobject-2.0'),
  dependency('gio-2.0'),
  dependency('libxml-2.0'),
  dependency('libarchive')
]

configure_file(
  output: 'config.h',
  configuration: config_h
)

gnome = import('gnome')
pkg = import('pkgconfig')

top_inc = include_directories('.')

subdir('libgepub')
subdir('tests')
