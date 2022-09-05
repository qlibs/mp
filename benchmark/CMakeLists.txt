#
# Copyright (c) 2022 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.18)
project(boost.mpl.benchmark)

set(CMAKE_CXX_FLAGS "-std=c++2b -stdlib=libc++ -fbracket-depth=1024 -ftemplate-depth=2048")

include(metabench/metabench.cmake)

metabench_add_dataset(sort_unique_reverse.mp "sort_unique_reverse/mp.cpp.erb"
    "(1...501).step(100)")
metabench_add_dataset(sort_unique_reverse.mp11
    "sort_unique_reverse/mp11.cpp.erb" "(1...501).step(100)")
metabench_add_dataset(sort_unique_reverse.circle
    "sort_unique_reverse/circle.cpp.erb" "(1...501).step(100)")

metabench_add_chart(sort_unique_reverse
    TITLE "sort_unique_reverse"
    SUBTITLE "(smaller is better)"
    XLABEL "Number of elements"
    YLABEL "Time"
    DATASETS
      sort_unique_reverse.mp
      sort_unique_reverse.mp11
      sort_unique_reverse.circle
)

metabench_add_dataset(transform_filter.mp "transform_filter/mp.cpp.erb"
    "(1...501).step(100)")
metabench_add_dataset(transform_filter.mp11 "transform_filter/mp11.cpp.erb"
    "(1...501).step(100)")

metabench_add_chart(transform_filter
    TITLE "transform_filter"
    SUBTITLE "(smaller is better)"
    XLABEL "Number of elements"
    YLABEL "Time"
    DATASETS
      transform_filter.mp
      transform_filter.mp11
)

metabench_add_dataset(conditional_drop_sum.mp "conditional_drop_sum/mp.cpp.erb"
    "(10...511).step(100)")
metabench_add_dataset(conditional_drop_sum.mp11 "conditional_drop_sum/mp11.cpp.erb"
    "(10...511).step(100)")

metabench_add_chart(conditional_drop_sum
    TITLE "conditional_drop_sum"
    SUBTITLE "(smaller is better)"
    XLABEL "Number of elements"
    YLABEL "Time"
    DATASETS
      conditional_drop_sum.mp
      conditional_drop_sum.mp11
)

metabench_add_dataset(first_or_last_size.mp "first_or_last_size/mp.cpp.erb"
    "(0...500).step(100)")
#metabench_add_dataset(first_or_last_size.mp11 "first_or_last_size/mp11.cpp.erb"
    #"(0...500).step(100)")

metabench_add_chart(first_or_last_size
    TITLE "first_or_last_size"
    SUBTITLE "(smaller is better)"
    XLABEL "Number of elements"
    YLABEL "Time"
    DATASETS
      first_or_last_size.mp
      #first_or_last_size.mp11
)
