#
# Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
function(example name)
  add_executable(${name}_example ${name}.cpp)
  target_include_directories(${name}_example PRIVATE ${CMAKE_SOURCE_DIR}/vendor/ut/include)
  target_link_libraries(${name}_example mp)
  add_test(NAME ${name}_example COMMAND ./${name}_example)
endfunction()

example(append)
example(conditional)
#example(count_if)
example(filter)
example(insert)
example(nth_pack_element)
example(reverse)
example(rotate)
example(slice)
example(sort)
example(split)
example(transform)
example(unique)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
example(to_list)
example(to_tuple)
endif()
