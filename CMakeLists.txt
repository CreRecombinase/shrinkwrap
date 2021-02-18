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



# if (BUILD_SHARED_LIBS)
#     set(LIBLZMA_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}lzma${CMAKE_SHARED_LIBRARY_SUFFIX})
#     set(ZLIB_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}z${CMAKE_SHARED_LIBRARY_SUFFIX})
#     set(ZSTD_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}zstd${CMAKE_SHARED_LIBRARY_SUFFIX})
# else()
#     set(LIBLZMA_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}lzma${CMAKE_STATIC_LIBRARY_SUFFIX})
#     set(ZLIB_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX})
#     set(ZSTD_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX})
#   endif()


FetchContent_Declare(
  zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG v1.4.8
  )
FetchContent_GetProperties(zstd)
if(NOT zstd_POPULATED)
  FetchContent_Populate(zstd)
  add_subdirectory(${zstd_SOURCE_DIR}/build/cmake EXCLUDE_FROM_ALL)
endif()


find_package(LIBLZMA REQUIRED)

find_package(ZLIB)



add_library(shrinkwrap INTERFACE)
add_subdirectory(include)
# target_sources(shrinkwrap INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/shrinkwrap/xz.hpp;${CMAKE_CURRENT_SOURCE_DIR}/include/shrinkwrap/gz.hpp;${CMAKE_CURRENT_SOURCE_DIR}/include/shrinkwrap/zstd.hpp;${CMAKE_CURRENT_SOURCE_DIR}/include/shrinkwrap/istream.hpp>)
# target_include_directories(shrinkwrap INTERFACE
#   $<INSTALL_INTERFACE:include>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(SHRINKWRAP_BUILD_TESTS)
  add_executable(shrinkwrap-test src/test.cpp)
  target_link_libraries(shrinkwrap-test shrinkwrap)
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

# install(DIRECTORY include/shrinkwrap DESTINATION include)
# if (CMAKE_VERSION VERSION_GREATER 3.3)
#     install(TARGETS shrinkwrap EXPORT ${PROJECT_NAME}-config
#             LIBRARY DESTINATION lib
#             ARCHIVE DESTINATION lib)

#     install(EXPORT ${PROJECT_NAME}-config DESTINATION share/${PROJECT_NAME})
#     write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake COMPATIBILITY SameMajorVersion)
#     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake DESTINATION share/${PROJECT_NAME})
#     export(EXPORT ${PROJECT_NAME}-config)
# endif()
