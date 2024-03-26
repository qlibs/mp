# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets hetero make fusion list        fusion.list.cpp.erb      1 "(0..50).step(10).to_a + (100..100).step(50).to_a")
add_dataset(datasets hetero make fusion map         fusion.map.cpp.erb       1 "(0..50).step(10).to_a + (100..100).step(50).to_a")
add_dataset(datasets hetero make fusion set         fusion.set.cpp.erb       2 "(0..50).step(10).to_a + (100..200).step(50).to_a")
add_dataset(datasets hetero make fusion vector      fusion.vector.cpp.erb    2 "(0..50).step(10).to_a + (100..200).step(50).to_a")
add_dataset(datasets hetero make hana   basic_tuple hana.basic_tuple.cpp.erb 2 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero make hana   map         hana.map.cpp.erb         2 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero make hana   set         hana.set.cpp.erb         2 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero make hana   tuple       hana.tuple.cpp.erb       2 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero make std    array       std.array.cpp.erb        2 "(0..50).step(10).to_a + (100..300).step(50).to_a")
add_dataset(datasets hetero make std    tuple       std.tuple.cpp.erb        2 "(0..50).step(10).to_a + (100..100).step(50).to_a")
add_dataset(datasets hetero make tmp    pack        tmp.pack.cpp.erb         2 "(0..50).step(10).to_a + (100..300).step(50).to_a")

add_chart(hetero make DATASETS ${datasets})
