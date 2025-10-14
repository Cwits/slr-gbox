# define a function that can be used to add tests automatically
function (create_test name)
  # each test creates its own executable
  add_executable(${name} ${name}.cpp)

  # we need to link each test separately against google test
  target_link_libraries(${name} gtest gtest_main slrlib)

  # include the root level directory for each executable so we can find header files relative to the root
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR})

  target_compile_options(${name} PRIVATE -g -O0 -Wall)

  # register the test with CTest, so we execute it automatically when running CTest later
  add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
endfunction()