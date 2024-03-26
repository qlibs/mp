# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets hetero find_if fusion list   fusion.list.cpp.erb   3 "(0..50).step(10)")
add_dataset(datasets hetero find_if fusion vector fusion.vector.cpp.erb 3 "(0..50).step(10).to_a + (100..100).step(50).to_a")
add_dataset(datasets hetero find_if hana   map    hana.map.cpp.erb      3 "(0..50).step(10).to_a + (100..150).step(50).to_a")
add_dataset(datasets hetero find_if hana   set    hana.set.cpp.erb      3 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero find_if hana   tuple  hana.tuple.cpp.erb    3 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero find_if tmp    pack  tmp.pack.cpp.erb       3 "(0..50).step(10).to_a + (100..300).step(50).to_a")

add_chart(hetero find_if DATASETS ${datasets})
