target_include_directories(shrinkwrap INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(shrinkwrap INTERFACE LibLZMA::LibLZMA ZLIB::ZLIB libzstd_static)
