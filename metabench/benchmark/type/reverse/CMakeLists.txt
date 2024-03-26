# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
#add_dataset(datasets type reverse brigand list   brigand.list.cpp.erb 60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type reverse kvasir  list   kvasir.list.cpp.erb  60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type reverse meta    list   meta.list.cpp.erb    60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type reverse metal   list   metal.list.cpp.erb   60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
add_dataset(datasets type reverse mp11    list   mp11.list.cpp.erb    60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
add_dataset(datasets type reverse mp    vector   mp.vector.cpp.erb    60 "(0..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type reverse mpl     list   mpl.list.cpp.erb     40 "(0..50).step(10)")
#add_dataset(datasets type reverse mpl     vector mpl.vector.cpp.erb   20 "(0..50).step(10)")

add_chart(type reverse DATASETS ${datasets})
