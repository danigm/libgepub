project(
  'libgepub', 'c',
  version: '0.5',
  license: 'LGPL2+',
  default_options: [
    'buildtype=debugoptimized',
    'c_std=gnu99',
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

# package
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('PACKAGE_BUGREPORT', 'danigm@wadobo.com')
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set_quoted('PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), gepub_version))
config_h.set_quoted('PACKAGE_TARNAME', meson.project_name())
config_h.set_quoted('PACKAGE_URL', '')
config_h.set_quoted('PACKAGE_VERSION', gepub_version)
config_h.set_quoted('PACKAGE_API_VERSION', gepub_version)
config_h.set_quoted('VERSION', gepub_version)

# headers
config_h.set('HAVE_DLFCN_H', cc.has_header('dlfcn.h'))
config_h.set('HAVE_INTTYPES_H', cc.has_header('inttypes.h'))
config_h.set('HAVE_MEMORY_H', cc.has_header('memory.h'))
config_h.set('HAVE_STDINT_H', cc.has_header('stdint.h'))
config_h.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
config_h.set('HAVE_STRINGS_H', cc.has_header('strings.h'))
config_h.set('HAVE_STRING_H', cc.has_header('string.h'))
config_h.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
config_h.set('HAVE_SYS_TYPES_H', cc.has_header('sys/types.h'))
config_h.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))

# compiler flags
common_flags = ['-DHAVE_CONFIG_H']

buildtype = get_option('buildtype')
if buildtype == 'debug' or buildtype == 'debugoptimized'
  test_cflags = [
    '-fno-strict-aliasing',
    '-Wcast-align',
    '-Wdeclaration-after-statement',
    '-Wformat-nonliteral',
    '-Wformat=2',
    '-Winit-self',
    '-Winline',
    '-Wmissing-declarations',
    '-Wmissing-format-attribute',
    '-Wmissing-include-dirs',
    '-Wmissing-noreturn',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wno-error=missing-field-initializers',
    '-Wno-error=unused-parameter',
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
    '-Wold-style-definition',
    '-Wpacked',
    '-Wredundant-decls',
    '-Wshadow',
    '-Wstrict-prototypes',
    '-Wswitch-enum',
    '-Wundef',
    '-Wunused-but-set-variable',
    '-Wwrite-strings'
  ]

  foreach cflag: test_cflags
    if cc.has_argument(cflag)
      common_flags += [cflag]
    endif
  endforeach
endif

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
