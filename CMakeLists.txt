#
# Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.12)
project(
  mp
  VERSION 0.0.1
  LANGUAGES CXX
)

set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME}-${PROJECT_VERSION}/include)
option(CMAKE_CXX_STANDARD_REQUIRED "Default value for CXX_STANDARD_REQUIRED property of targets." YES)
option(CMAKE_CXX_EXTENSIONS "Default value for CXX_EXTENSIONS property of targets." NO)
option(BOOST_MP_BUILD_EXAMPLES "Build the examples" ${MASTER_PROJECT})
option(BOOST_MP_BUILD_TESTS "Build the tests" ${MASTER_PROJECT})
option(BOOST_MP_DISABLE_MODULE "Disable mp module" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_library(mp INTERFACE)
target_include_directories(mp INTERFACE ${CMAKE_SOURCE_DIR}/include)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
target_compile_options(mp INTERFACE -stdlib=libc++ -Wall -Wextra -Wpedantic -Werror)
target_link_options(mp INTERFACE -stdlib=libc++)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
target_compile_options(mp INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(style)
add_custom_command(
  TARGET style COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example ${CMAKE_CURRENT_LIST_DIR}/include
                       ${CMAKE_CURRENT_LIST_DIR}/test -iname "*.hpp" -or -iname "*.cpp" | xargs clang-format -i
)

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
