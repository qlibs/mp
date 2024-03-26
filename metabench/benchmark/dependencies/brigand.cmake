# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

if (METABENCH_BRIGAND)
    find_package(Brigand QUIET)
    if (Brigand_FOUND)
        message(STATUS "Local Brigand installation found - version ${Brigand_VERSION}")
        add_custom_target(Brigand)
    else()
        message(STATUS "No local Brigand installation found - fetching branch master")
        include(ExternalProject)
        ExternalProject_Add(Brigand EXCLUDE_FROM_ALL 1
            URL https://github.com/edouarda/brigand/archive/master.zip
            TIMEOUT 120
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/brigand"
            CONFIGURE_COMMAND "" # Disable configure step
            BUILD_COMMAND ""     # Disable build step
            INSTALL_COMMAND ""   # Disable install step
            TEST_COMMAND ""      # Disable test step
            UPDATE_COMMAND ""    # Disable source work-tree update
        )
        ExternalProject_Get_Property(Brigand SOURCE_DIR)
        set(Brigand_INCLUDE_DIRS ${SOURCE_DIR}/include)
    endif()

    function(Brigand_add_dataset dataset datatype)
        set(color "hsl(207, 54%, 47%)")
        metabench_add_dataset(${dataset} ${ARGN} COLOR ${color})
        target_include_directories(${dataset} PUBLIC ${Brigand_INCLUDE_DIRS})
        add_dependencies(${dataset} Brigand)
    endfunction()
else()
    function(Brigand_add_dataset)
    endfunction()
endif()
