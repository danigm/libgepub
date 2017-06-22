headers = files(
  'gepub-archive.h',
  'gepub-doc.h',
  'gepub-text-chunk.h',
  'gepub-widget.h',
  'gepub.h'
)

install_headers(
  headers,
  subdir: gepub_pkgincludedir
)

private_headers = files(
  'gepub-utils.h'
)

sources = files(
  'gepub-archive.c',
  'gepub-doc.c',
  'gepub-text-chunk.c',
  'gepub-utils.c',
  'gepub-widget.c'
)

symbol_map = 'gepub.map'

ldflags = [
  '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbol_map)
]

gepub_ldflags = []

if host_machine.system().contains('linux')
  foreach ldflag: ldflags
    if cc.has_argument(ldflag)
      gepub_ldflags += ldflag
    endif
  endforeach
endif

libgepub = library(
  'gepub',
  sources: sources,
  version: libversion,
  soversion: soversion,
  include_directories: top_inc,
  dependencies: gepub_deps,
  link_args: gepub_ldflags,
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
  variables: [
    'exec_prefix=' + gepub_libexecdir
  ],
  install_dir: join_paths(gepub_libdir, 'pkgconfig')
)

if get_option('enable-introspection') and get_option('default_library') == 'shared'
  gir_inc = [
    'GObject-2.0',
    'libxml2-2.0',
    'WebKit2-4.0'
  ]

  gir_extra_args = [
    '--warn-all'
  ]

  gir_dir = join_paths(gepub_datadir, '@0@-@1@'.format('gir', gepub_gir_version))
  typelib_dir = join_paths(gepub_libdir, '@0@-@1@'.format('girepository', gepub_gir_version))

  gnome.generate_gir(
    libgepub,
    sources: sources + headers + private_headers,
    namespace: gepub_gir_ns,
    nsversion: gepub_version,
    identifier_prefix: gepub_gir_ns,
    symbol_prefix: gepub_gir_ns.to_lower(),
    includes: gir_inc,
    include_directories: top_inc,
    install: true,
    install_dir_gir: gir_dir,
    install_dir_typelib: typelib_dir,
    extra_args: gir_extra_args
  )
endif
