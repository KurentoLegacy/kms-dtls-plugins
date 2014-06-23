cmake_minimum_required(VERSION 2.8)

project ("kms-dtls-plugins")

message ("If KurentoHelpers is not found, then it is necessary to install
          kms-cmake-utils package from the kurento repository")
find_package(KurentoHelpers REQUIRED)

include (KurentoGitHelpers)

get_git_version (PROJECT_VERSION 1.0.9-dev)
install_git_hook (pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)

find_package(PkgConfig)

set (GST_REQUIRED 1.3.3)
set (GLIB_REQUIRED 2.38)
set (GNUTLS_REQUIRED 3.2.0)
set (SOUP_REQUIRED 2.40)
set (NICE_REQUIRED 0.1.4)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=${GST_REQUIRED})
pkg_check_modules(GSTREAMER_CHECK REQUIRED gstreamer-check-1.0>=${GST_REQUIRED})
pkg_check_modules(GSTREAMER_RTP REQUIRED gstreamer-rtp-1.0>=${GST_REQUIRED})
pkg_check_modules(GIO REQUIRED gio-2.0>=${GLIB_REQUIRED})
pkg_check_modules(GNUTLS REQUIRED gnutls>=${GNUTLS_REQUIRED})
pkg_check_modules(SOUP libsoup-2.4>=${SOUP_REQUIRED})
pkg_check_modules(NICE nice>=${NICE_REQUIRED})

find_program(VALGRIND valgrind)

set (VERSION ${PROJECT_VERSION})
set (PACKAGE ${PROJECT_NAME})
set (GETTEXT_PACKAGE "kms-dtls")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(GNUInstallDirs)

set (CMAKE_INSTALL_MODULESDIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DHAVE_CONFIG_H -Werror -Wall")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (DISABLE_TESTS FALSE CACHE BOOL "Disable make check")

enable_testing()

add_subdirectory(src)

add_subdirectory(tests)

message(STATUS "Compiling : ${PROJECT_NAME}-${PROJECT_VERSION}")
