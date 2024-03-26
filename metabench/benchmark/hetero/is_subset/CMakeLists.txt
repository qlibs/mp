# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets hetero is_subset hana map   hana.map.cpp.erb   3 "(0..50).step(10).to_a")
add_dataset(datasets hetero is_subset hana set   hana.set.cpp.erb   3 "(0..50).step(10).to_a")
add_dataset(datasets hetero is_subset hana tuple hana.tuple.cpp.erb 3 "(0..50).step(10).to_a")

add_chart(hetero is_subset DATASETS ${datasets})
