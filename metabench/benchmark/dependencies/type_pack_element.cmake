# Copyright Odin Holmes 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_Ptype_pack_element
    AND NOT (CMAKE_CXX_COPtype_pack_elementILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COPtype_pack_elementILER_VERSION VERSION_LESS "4.7")
    AND NOT (CMAKE_CXX_COPtype_pack_elementILER_ID STREQUAL "Clang" AND
             CMAKE_CXX_COPtype_pack_elementILER_VERSION VERSION_LESS "3.4")
    AND NOT (CMAKE_CXX_COPtype_pack_elementILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COPtype_pack_elementILER_VERSION VERSION_LESS "12")
)
    function(Ptype_pack_element_add_dataset dataset datatype)
        set(color "hsl(60, 100%, 50%)") # yellow
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        add_dependencies(${dataset} ptype_pack_element)
    endfunction()
endif()

if (NOT COMMAND Ptype_pack_element_add_dataset)
    function(Ptype_pack_element_add_dataset)
    endfunction()
endif()
