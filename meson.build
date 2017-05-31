project(
  'libgepub', 'c',
  version: '0.4',
  license: 'LGPL2+',
  default_options: [
    'buildtype=debugoptimized',
    'c_std=gnu99',
    'warning_level=1'
  ],
  meson_version: '>= 0.38.1'
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
gepub_includedir = join_paths(gepub_prefix, get_option('includedir'))

gepub_pkgincludedir = join_paths(gepub_includedir, meson.project_name())

soversion = 0
current = 0
revision = 0
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

cc = meson.get_compiler('c')

conf = configuration_data()

# package
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_BUGREPORT', 'danigm@wadobo.com')
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), gepub_version))
conf.set_quoted('PACKAGE_TARNAME', meson.project_name())
conf.set_quoted('PACKAGE_URL', '')
conf.set_quoted('PACKAGE_VERSION', gepub_version)
conf.set_quoted('PACKAGE_API_VERSION', gepub_version)
conf.set_quoted('VERSION', gepub_version)

# headers
conf.set('HAVE_DLFCN_H', cc.has_header('dlfcn.h'))
conf.set('HAVE_INTTYPES_H', cc.has_header('inttypes.h'))
conf.set('HAVE_MEMORY_H', cc.has_header('memory.h'))
conf.set('HAVE_STDINT_H', cc.has_header('stdint.h'))
conf.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
conf.set('HAVE_STRINGS_H', cc.has_header('strings.h'))
conf.set('HAVE_STRING_H', cc.has_header('string.h'))
conf.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
conf.set('HAVE_SYS_TYPES_H', cc.has_header('sys/types.h'))
conf.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))

# Compiler flags
test_cflags = [
  '-fno-strict-aliasing',
  '-Wall',
  '-Warray-bounds',
  '-Wcast-align',
  '-Wdeclaration-after-statement',
  '-Wextra',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Wformat=2',
  '-Wimplicit-function-declaration',
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
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wreturn-type',
  '-Wshadow',
  '-Wsign-compare',
  '-Wstrict-aliasing',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wundef',
  '-Wunused-but-set-variable',
  '-Wwrite-strings'
]

common_flags = [
  '-DHAVE_CONFIG_H'
]

foreach cflag: test_cflags
  if cc.has_argument(cflag)
    common_flags += [cflag]
  endif
endforeach

add_global_arguments(common_flags, language: 'c')

gepub_dep = [
  dependency('webkit2gtk-4.0'),
  dependency('libsoup-2.4'),
  dependency('glib-2.0'),
  dependency('gobject-2.0'),
  dependency('gio-2.0'),
  dependency('libxml-2.0'),
  dependency('libarchive')
]

configure_file(
  output : 'config.h',
  configuration : conf
)

gnome = import('gnome')
pkg = import('pkgconfig')

top_inc = include_directories('.')

subdir('libgepub')
subdir('tests')
