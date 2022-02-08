cmake_minimum_required(VERSION 3.21)
project(libsouptest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/libsoup@2/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}://usr/local/opt/icu4c/lib/pkgconfig")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(LIBSOUP REQUIRED IMPORTED_TARGET libsoup-2.4)

add_executable(libsouptest main.cpp)
target_link_libraries(libsouptest PkgConfig::GLIB)
target_link_libraries(libsouptest PkgConfig::LIBSOUP)
target_include_directories(libsouptest PRIVATE ${LIBSOUP_INCLUDE_DIRS})
target_include_directories(libsouptest PRIVATE ${GLIB_INCLUDE_DIRS})
