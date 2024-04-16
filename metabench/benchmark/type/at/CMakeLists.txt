# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
#add_dataset(datasets type at brigand list   brigand.list.cpp.erb 1 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a" "p\;100")
#add_dataset(datasets type at hana    types  hana.types.cpp.erb   1 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a" "p\;100")
#add_dataset(datasets type at kvasir  list   kvasir.list.cpp.erb  1 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a" "p\;100")
#add_dataset(datasets type at meta    list   meta.list.cpp.erb    1 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a" "p\;100")
#add_dataset(datasets type at metal   list   metal.list.cpp.erb   1 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a" "p\;100")
add_dataset(datasets type at mp11    list   mp11.list.cpp.erb    1 "[1] + (10..100).step(10).to_a" "p\;100")
add_dataset(datasets type at mp    vector   mp.vector.cpp.erb    1 "[1] + (10..100).step(10).to_a" "p\;100")
add_dataset(datasets type at p2996    vector   p2996.vector.cpp.erb    1 "[1] + (10..100).step(10).to_a" "p\;100")
#add_dataset(datasets type at mpl     list   mpl.list.cpp.erb     1 "[1] + (10..50).step(10).to_a"                            "p\;100")
#add_dataset(datasets type at mpl     vector mpl.vector.cpp.erb   1 "[1] + (10..50).step(10).to_a"                            "p\;100")

add_chart(type at DATASETS ${datasets})
