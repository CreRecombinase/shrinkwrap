cmake_minimum_required(VERSION 3.15)
project(shrinkwrap VERSION 1.0.0 LANGUAGES CXX)
include(FetchContent)
include(ExternalProject)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)


set(CMAKE_CXX_STANDARD 11)

option(SHRINKWRAP_BUILD_TESTS "Set to ON to build tests" ON)
if(SHRINKWRAP_BUILD_TESTS)
  enable_testing()
endif()


set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_BUILD_PROGRAMS OFF)

FetchContent_Declare(
  zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG v1.4.8
  SOURCE_SUBDIR "build/cmake"
  )
FetchContent_MakeAvailable(zstd)


find_package(LibLZMA REQUIRED)
find_package(ZLIB)

set(Shrinkwrap_headers
  include/shrinkwrap/xz.hpp
  include/shrinkwrap/zstd.hpp
  include/shrinkwrap/gz.hpp
  include/shrinkwrap/istream.hpp)

add_library(shrinkwrap INTERFACE ${Shrinkwrap_headers})

target_include_directories(shrinkwrap INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(shrinkwrap INTERFACE LibLZMA::LibLZMA ZLIB::ZLIB libzstd_static)

if(SHRINKWRAP_BUILD_TESTS)
  add_executable(shrinkwrap-test src/test.cpp)
  target_link_libraries(shrinkwrap-test PRIVATE shrinkwrap)
endif()

add_test(xz_seek_test shrinkwrap-test xz-seek)
add_test(xz_iterator_test shrinkwrap-test xz-iter)
add_test(gz_iterator_test shrinkwrap-test gz-iter)
add_test(bgzf_seek_test shrinkwrap-test bgzf-seek)
add_test(bgzf_iterator_test shrinkwrap-test bgzf-iter)
add_test(zstd_iterator_test shrinkwrap-test zstd-iter)
add_test(zstd_seek_test shrinkwrap-test zstd-seek)
add_test(generic_iterator_test shrinkwrap-test generic-iter)
add_test(generic_seek_test shrinkwrap-test generic-seek)


install(TARGETS shrinkwrap EXPORT Shrinkwrap)
install(EXPORT Shrinkwrap NAMESPACE shrinkwrap::
  DESTINATION lib/cmake/shrinkwrap)
install(FILES ${Shrinkwrap_headers} DESTINATION
  include/shrinkwrap)
