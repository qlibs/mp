# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

set(datasets)
add_dataset(datasets type at p2996                vector   p2996.vector.cpp.erb    1 "[10] + (5..100).step(5).to_a" "p\;100")
add_dataset(datasets type at mp                   vector   mp.vector.cpp.erb    1 "[10] + (5..100).step(5).to_a" "p\;100")
add_dataset(datasets type at mp11                 list     mp11.list.cpp.erb    1 "[10] + (5..100).step(5).to_a" "p\;100")
add_chart(type at DATASETS ${datasets})
