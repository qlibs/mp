# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_META AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
    find_package(Meta QUIET)
    if (Meta_FOUND)
        message(STATUS "Local Meta installation found - version ${Meta_VERSION}")
        add_custom_target(Meta)
    else()
        message(STATUS "No local Meta installation found - fetching branch master")
        include(ExternalProject)
        ExternalProject_Add(Meta EXCLUDE_FROM_ALL 1
            URL https://github.com/ericniebler/meta/archive/master.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/meta"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(Meta SOURCE_DIR)
        set(Meta_INCLUDE_DIRS ${SOURCE_DIR}/include)
    endif()

    function(Meta_add_dataset dataset datatype)
        set(color "hsl(118, 41%, 49%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Meta_INCLUDE_DIRS})
        add_dependencies(${dataset} Meta)
    endfunction()
else()
    function(Meta_add_dataset)
    endfunction()
endif()
