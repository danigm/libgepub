headers = [
  'gepub-double-widget.h',
  'gepub-widget.h',
  'gepub-archive.h',
  'gepub-text-chunk.h',
  'gepub-doc.h',
  'gepub.h'
]

install_headers(
  headers,
  subdir: gepub_pkgincludedir
)

private_headers = [
  'gepub-utils.h'
]

sources = [
  'gepub-double-widget.c',
  'gepub-widget.c',
  'gepub-archive.c',
  'gepub-text-chunk.c',
  'gepub-doc.c',
  'gepub-utils.c'
]

ldflags = [
  '-no-undefined',
  '-export-symbols-regex "^gepub_*"'
]

gepub_ldflags = []

if host_machine.system().contains('linux')
  foreach ldflag: ldflags
    if cc.has_argument(ldflag)
      gepub_ldflags += ldflag
    endif
  endforeach
endif

libtype = get_option('default_library')

if libtype == 'static'
  libgepub = static_library(
    'gepub',
    sources: sources + headers + private_headers,
    include_directories: top_inc,
    dependencies: gepub_dep,
    link_args: gepub_ldflags,
    install: true,
    install_dir: gepub_libdir
  )
else
  libgepub = shared_library(
    'gepub',
    sources: sources + headers + private_headers,
    version: libversion,
    soversion: soversion,
    include_directories: top_inc,
    dependencies: gepub_dep,
    link_args: gepub_ldflags,
    install: true,
    install_dir: gepub_libdir
  )
endif

libgepub_dep = declare_dependency(
  link_with: libgepub,
  include_directories: include_directories('.'),
  dependencies: gepub_dep
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
  ]
)

if get_option('enable-introspection') and libtype == 'shared'
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
