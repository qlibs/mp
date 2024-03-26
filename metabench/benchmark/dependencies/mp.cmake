# Copyright Odin Holmes 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_MP
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
)
    function(MP_add_dataset dataset datatype)
        set(color "hsl(118, 41%, 49%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        add_dependencies(${dataset} mp11)
    endfunction()
endif()

if (NOT COMMAND MP_add_dataset)
    function(MP_add_dataset)
    endfunction()
endif()
