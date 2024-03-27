# Copyright Bruno Dutra 2016
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
#add_dataset(datasets type insert kvasir list   kvasir.list.cpp.erb 40 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type insert metal  list   metal.list.cpp.erb  40 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
#add_dataset(datasets type insert mpl    list   mpl.list.cpp.erb    10 "[1] + (10..50).step(10).to_a")
add_dataset(datasets type insert mp11   list   mp11.list.cpp.erb   40 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")
add_dataset(datasets type insert mp   vector   mp.vector.cpp.erb   40 "[1] + (10..50).step(10).to_a + (100..500).step(50).to_a")

add_chart(type insert DATASETS ${datasets})
