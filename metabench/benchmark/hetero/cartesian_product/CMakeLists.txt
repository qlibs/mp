# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets hetero cartesian_product hana tuple hana.tuple.cpp.erb 1 "(0..30).step(5)")

add_chart(hetero cartesian_product DATASETS ${datasets})
