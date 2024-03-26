# Copyright Odin Holmes 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_MP11
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
)
    find_package(Boost QUIET)
    if (Boost_FOUND)
        message(STATUS "No local mp11 installation found - fetching branch master")
        include(ExternalProject)
        ExternalProject_Add(mp11 EXCLUDE_FROM_ALL 1
            URL https://github.com/pdimov/mp11/archive/master.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/mp11"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(mp11 SOURCE_DIR)
        set(MP11_INCLUDE_DIR ${SOURCE_DIR}/include)

        function(Mp11_add_dataset dataset datatype)
            set(color "hsl(54, 42%, 41%)")
            metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
            target_include_directories(${dataset} PUBLIC ${MP11_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
            add_dependencies(${dataset} mp11)
        endfunction()
    else()
        message(STATUS "No local Boost installation found - mp11 will be unavailable.")
    endif()
endif()

if (NOT COMMAND Mp11_add_dataset)
    function(Mp11_add_dataset)
    endfunction()
endif()
