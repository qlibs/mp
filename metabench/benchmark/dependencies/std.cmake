# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19")
    message(STATUS "The compiler has an unsupported standard library - standard library will be unavailable.")

    function(Std_add_dataset)
    endfunction()
else()
    function(Std_add_dataset dataset datatype)
        if("${datatype}" STREQUAL "tuple")
            set(color "hsl(118, 41%, 49%)")
        elseif("${datatype}" STREQUAL "array")
            set(color "hsl(118, 41%, 69%)")
        else()
            message(FATAL_ERROR "Unknown datatype '${datatype}' in the standard library")
        endif()
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
    endfunction()
endif()
