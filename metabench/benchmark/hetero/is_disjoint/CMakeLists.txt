# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets hetero is_disjoint hana set   hana.set.cpp.erb   1 "(0..50).step(10)")
add_dataset(datasets hetero is_disjoint hana map   hana.map.cpp.erb   1 "(0..50).step(10)")
add_dataset(datasets hetero is_disjoint hana tuple hana.tuple.cpp.erb 1 "(0..50).step(10)")

add_chart(hetero is_disjoint DATASETS ${datasets})
