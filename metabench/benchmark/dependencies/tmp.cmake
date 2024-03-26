# Copyright Odin Holmes 2018
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_TMP
        AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5")
        AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.6")
        AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19")
        )
    message(STATUS "No local TMP installation found - fetching branch development")
    include(ExternalProject)
    ExternalProject_Add(Tmp EXCLUDE_FROM_ALL 1
            URL https://github.com/odinthenerd/tmp/archive/development.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/tmp"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
            )
    ExternalProject_Get_Property(Tmp SOURCE_DIR)
    set(Tmp_INCLUDE_DIR ${SOURCE_DIR}/include)

    function(Tmp_add_dataset dataset datatype)
        set(color "hsl(293, 100%, 43%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Tmp_INCLUDE_DIR})
        add_dependencies(${dataset} Tmp)
    endfunction()
else()
    function(Tmp_add_dataset)
    endfunction()
endif()
