# Copyright Bruno Dutra 2016
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
#add_dataset(datasets type erase brigand list   brigand.list.cpp.erb 60 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type erase kvasir  list   kvasir.list.cpp.erb  60 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type erase metal   list   metal.list.cpp.erb   60 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
add_dataset(datasets type erase mp11    list   mp11.list.cpp.erb    60 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
add_dataset(datasets type erase mp    list   mp.vector.cpp.erb    60 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type erase mpl     list   mpl.list.cpp.erb     12 "[1] + (10..50).step(10).to_a")
#add_dataset(datasets type erase mpl     vector mpl.vector.cpp.erb   12 "[1] + (10..50).step(10).to_a")

add_chart(type erase DATASETS ${datasets})
