gepub_tests = [
  'test-gepub'
]

gepub_test_dep = [
  gepub_dep,
  libgepub_dep,
  dependency('gtk+-3.0')
]

foreach gepub_test: gepub_tests
  exe = executable(
    gepub_test,
    '@0@.c'.format(gepub_test),
    include_directories: top_inc,
    dependencies: gepub_test_dep
  )

  test(gepub_test, exe)
endforeach
