# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_HANA
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
)
    find_package(Hana QUIET)
    if (Hana_FOUND)
        message(STATUS "Local Hana installation found - version ${Hana_VERSION}")
        add_custom_target(Hana)
    else()
        message(STATUS "No local Hana installation found - fetching branch develop")
        include(ExternalProject)
        ExternalProject_Add(Hana EXCLUDE_FROM_ALL 1
            URL https://github.com/boostorg/hana/archive/develop.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/hana"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(Hana SOURCE_DIR)
        set(Hana_INCLUDE_DIRS ${SOURCE_DIR}/include)
    endif()

    function(Hana_add_dataset dataset datatype)
        if(${datatype} MATCHES "^(types|tuple|std_integer_sequence)$")
            set(color "hsl(30, 100%, 50%)")
        elseif("${datatype}" STREQUAL "basic_tuple")
            set(color "hsl(30, 100%, 62%)")
        elseif("${datatype}" STREQUAL "set")
            set(color "hsl(30, 100%, 74%)")
        elseif("${datatype}" STREQUAL "map")
            set(color "hsl(30, 100%, 86%)")
        else()
            message(FATAL_ERROR "Unknown datatype '${datatype}' in 'Hana'")
        endif()
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Hana_INCLUDE_DIRS})
        add_dependencies(${dataset} Hana)
    endfunction()
else()
    function(Hana_add_dataset)
    endfunction()
endif()
