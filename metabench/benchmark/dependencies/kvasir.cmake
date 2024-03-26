# Copyright Odin Holmes 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_KVASIR
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.8")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19")
)
        message(STATUS "No local Kvasir installation found - fetching branch development")
        include(ExternalProject)
        ExternalProject_Add(Kvasir EXCLUDE_FROM_ALL 1
            URL https://github.com/kvasir-io/mpl/archive/development.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/kvasir"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(Kvasir SOURCE_DIR)
        set(Kvasir_INCLUDE_DIR ${SOURCE_DIR}/src)

    function(Kvasir_add_dataset dataset datatype)
        set(color "hsl(0, 0%, 0%)") 
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Kvasir_INCLUDE_DIR})
        add_dependencies(${dataset} Kvasir)
    endfunction()
else()
    function(Kvasir_add_dataset)
    endfunction()
endif()
