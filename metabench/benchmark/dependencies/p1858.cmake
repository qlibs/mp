# Copyright Odin Holmes 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_P1858
    AND NOT (CMAKE_CXX_COP1858ILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COP1858ILER_VERSION VERSION_LESS "4.7")
    AND NOT (CMAKE_CXX_COP1858ILER_ID STREQUAL "Clang" AND
             CMAKE_CXX_COP1858ILER_VERSION VERSION_LESS "3.4")
    AND NOT (CMAKE_CXX_COP1858ILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COP1858ILER_VERSION VERSION_LESS "12")
)
    function(P1858_add_dataset dataset datatype)
        set(color "hsl(0, 0%, 0%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        add_dependencies(${dataset} p1858)
    endfunction()
endif()

if (NOT COMMAND P1858_add_dataset)
    function(P1858_add_dataset)
    endfunction()
endif()
