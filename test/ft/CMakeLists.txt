#
# Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
function(ft name)
  add_executable(${name}_test ${name}.cpp)
  target_include_directories(${name}_test PRIVATE ${CMAKE_SOURCE_DIR}/vendor/ut/include)
  target_compile_options(${name}_test PRIVATE "-D$(...)=::boost::mp::list<__VA_ARGS__>()")
  target_link_libraries(${name}_test mp)
  add_test(NAME ${name}_test COMMAND ./${name}_test)
endfunction()

ft(tuple)
ft(type_list)
ft(value_list)
