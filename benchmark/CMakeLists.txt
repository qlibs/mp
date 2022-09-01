cmake_minimum_required(VERSION 3.18)
project(boost.mpl.benchmark)

set(CMAKE_CXX_FLAGS "-std=c++2b -stdlib=libc++ -fbracket-depth=1024 -ftemplate-depth=2048")

include(metabench/metabench.cmake)

metabench_add_dataset(sort_unique_reverse.mp "sort_unique_reverse/mp.cpp.erb"
    "(1...501).step(100)")
metabench_add_dataset(sort_unique_reverse.mp11
    "sort_unique_reverse/mp11.cpp.erb" "(1...501).step(100)")

metabench_add_chart(sort_unique_reverse
    TITLE "sort_unique_reverse"
    SUBTITLE "(smaller is better)"
    XLABEL "Number of elements"
    YLABEL "Time"
    DATASETS
      sort_unique_reverse.mp
      sort_unique_reverse.mp11
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
