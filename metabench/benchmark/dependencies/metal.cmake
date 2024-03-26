# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_METAL
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5")
    AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
             CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19")
)
    find_package(Metal QUIET)
    if (Metal_FOUND)
        message(STATUS "Local Metal installation found - version ${Metal_VERSION}")
        get_target_property(Metal_INCLUDE_DIR Metal INTERFACE_INCLUDE_DIRECTORIES)
    else()
        message(STATUS "No local Metal installation found - fetching branch master")
        include(ExternalProject)
        ExternalProject_Add(Metal EXCLUDE_FROM_ALL 1
            URL https://github.com/brunocodutra/metal/archive/master.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/metal"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(Metal SOURCE_DIR)
        set(Metal_INCLUDE_DIR ${SOURCE_DIR}/include)
    endif()

    function(Metal_add_dataset dataset datatype)
        set(color "hsl(359, 80%, 50%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Metal_INCLUDE_DIR})
        add_dependencies(${dataset} Metal)
    endfunction()
else()
    function(Metal_add_dataset)
    endfunction()
endif()
