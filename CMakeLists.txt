#
# Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.23)
project(
  mp
  VERSION 0.0.1
  LANGUAGES CXX
)

option(BOOST_MP_BUILD_EXAMPLES "Build the examples" OFF)
option(BOOST_MP_BUILD_TESTS "Build the tests" OFF)
option(BOOST_MP_DISABLE_MODULE "Disable mp module" OFF)

if(PROJECT_IS_TOP_LEVEL)
  option(CMAKE_CXX_EXTENSIONS "Default value for CXX_EXTENSIONS target property." OFF)
endif()

add_library(mp INTERFACE)
add_library(Boost::mp ALIAS mp)
target_compile_features(mp INTERFACE cxx_std_20)
target_compile_definitions(mp
  INTERFACE $<INSTALL_INTERFACE:BOOST_MP_DISABLE_MODULE>)
target_sources(mp
  INTERFACE
  FILE_SET mp_headers
  TYPE HEADERS
  BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
  FILES "${PROJECT_SOURCE_DIR}/include/boost/mp.hpp")

if(PROJECT_IS_TOP_LEVEL)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(mp INTERFACE
      $<BUILD_INTERFACE:-stdlib=libc++ -Wall -Wextra -Wpedantic -Werror>)
    target_link_options(mp INTERFACE $<BUILD_INTERFACE:-stdlib=libc++>)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(mp INTERFACE $<BUILD_INTERFACE:-Wall -Wextra -Wpedantic -Werror>)
  endif()
endif()

if(BOOST_MP_DISABLE_MODULE)
  target_compile_definitions(mp INTERFACE BOOST_MP_DISABLE_MODULE)
endif()

if(BOOST_MP_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(BOOST_MP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

include("cmake/mpInstall.cmake")
include("cmake/mpStyleTarget.cmake")
mp_create_style_target(style)
