headers = files(
  'gepub-archive.h',
  'gepub-doc.h',
  'gepub-text-chunk.h',
  'gepub-widget.h',
  'gepub.h'
)

install_headers(
  headers,
  subdir: meson.project_name()
)

private_headers = files('gepub-utils.h')

sources = files(
  'gepub-archive.c',
  'gepub-doc.c',
  'gepub-text-chunk.c',
  'gepub-utils.c',
  'gepub-widget.c'
)

symbol_map = join_paths(meson.current_souce_dir(), 'gepub.map')

test_ldflag = '-Wl,--version-script,' + symbol_map
ldflags = []

if cc.has_argument(test_ldflag)
  ldflags += test_ldflag
endif

libgepub = library(
  'gepub',
  sources: sources,
  version: libversion,
  soversion: soversion,
  include_directories: top_inc,
  dependencies: gepub_deps,
  link_args: ldflags,
  link_depends: symbol_map,
  install: true,
  install_dir: gepub_libdir
)

libgepub_dep = declare_dependency(
  link_with: libgepub,
  include_directories: include_directories('.'),
  dependencies: gepub_deps
)

pkg.generate(
  libraries: libgepub,
  version: gepub_version,
  name: meson.project_name(),
  description: 'epub Documents library',
  filebase: meson.project_name(),
  subdirs: meson.project_name(),
  requires: 'gio-2.0',
  requires_private: [
    'libxml-2.0',
    'libarchive'
  ],
  variables: 'exec_prefix=' + gepub_libexecdir,
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)

if get_option('introspection') and get_option('default_library') == 'shared'
  gir_incs = [
    'GObject-2.0',
    'libxml2-2.0',
    'WebKit2-4.0'
  ]

  gir_extra_args = '--warn-all'

  gir_dir = join_paths(gepub_datadir, '@0@-@1@'.format('gir', gepub_gir_version))
  typelib_dir = join_paths(gepub_libdir, '@0@-@1@'.format('girepository', gepub_gir_version))

  gnome.generate_gir(
    libgepub,
    sources: sources + headers + private_headers,
    namespace: gepub_gir_ns,
    nsversion: gepub_api_version,
    includes: gir_incs,
    extra_args: gir_extra_args,
    install: true,
    install_dir_gir: gir_dir,
    install_dir_typelib: typelib_dir
  )
endif
