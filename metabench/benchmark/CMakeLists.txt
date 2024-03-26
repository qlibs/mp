# Copyright Louis Dionne 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

option(METABENCH_BRIGAND    "Benchmark Brigand" ON)
option(METABENCH_FUSION     "Benchmark Fusion"  ON)
option(METABENCH_HANA       "Benchmark Hana"    ON)
option(METABENCH_KVASIR     "Benchmark Kvasir"  ON)
option(METABENCH_TMP        "Benchmark Tmp"     ON)
option(METABENCH_META       "Benchmark Meta"    ON)
option(METABENCH_METAL      "Benchmark Metal"   ON)
option(METABENCH_MP11       "Benchmark mp11"    ON)
option(METABENCH_MPL        "Benchmark MPL"     ON)
option(METABENCH_MP         "Benchmark MP"      ON)

include(benchmark/dependencies/brigand)
include(benchmark/dependencies/fusion)
include(benchmark/dependencies/hana)
include(benchmark/dependencies/kvasir)
include(benchmark/dependencies/tmp)
include(benchmark/dependencies/meta)
include(benchmark/dependencies/metal)
include(benchmark/dependencies/mpl)
include(benchmark/dependencies/mp11)
include(benchmark/dependencies/mp)
include(benchmark/dependencies/std)

macro(add_dataset datasets category case lib datatype path_to_template k range)
    set(env "{k: ${k}")
    foreach(pair ${ARGN})
        list(GET pair 0 key)
        list(GET pair 1 val)
        set(env "${env}, ${key}: ${val}")
    endforeach()
    set(env "${env}}")

    set(name "${lib}.${datatype}")
    set(dataset "dataset.${category}.${case}.${name}")
    set(args ${dataset} ${datatype} ${path_to_template} ${range}
        NAME ${name} OUTPUT ${name}.json SCALE "1.0/(${k})" ENV ${env}
    )

    if("${lib}" STREQUAL "brigand")
        Brigand_add_dataset(${args})
    elseif("${lib}" STREQUAL "fusion")
        Fusion_add_dataset(${args})
    elseif("${lib}" STREQUAL "hana")
        Hana_add_dataset(${args})
    elseif("${lib}" STREQUAL "kvasir")
        Kvasir_add_dataset(${args})
    elseif("${lib}" STREQUAL "tmp")
        Tmp_add_dataset(${args})
    elseif("${lib}" STREQUAL "meta")
        Meta_add_dataset(${args})
    elseif("${lib}" STREQUAL "metal")
        Metal_add_dataset(${args})
    elseif ("${lib}" STREQUAL "mpl")
        MPL_add_dataset(${args})
    elseif ("${lib}" STREQUAL "mp11")
        Mp11_add_dataset(${args})
    elseif ("${lib}" STREQUAL "mp")
        MP_add_dataset(${args})
    elseif ("${lib}" STREQUAL "std")
        Std_add_dataset(${args})
    else()
        message(FATAL_ERROR "The library '${lib}' is currently not supported.")
    endif()

    if (TARGET ${dataset})
        list(APPEND ${datasets} ${dataset})
    endif()
endmacro()

add_custom_target(benchmark)
macro(add_chart category case)
    cmake_parse_arguments(ARGS "" "" "DATASETS" ${ARGN})

    if (NOT TARGET benchmark.${category})
        add_custom_target(benchmark.${category})
        add_dependencies(benchmark benchmark.${category})
    endif()

    # If we have no datasets, perhaps they were all disabled in the current
    # configuration. In this case, we omit the benchmark instead of failing.
    list(LENGTH ARGS_DATASETS _n_datasets)
    if (_n_datasets GREATER 0)
        metabench_add_chart(benchmark.${category}.${case} ${ARGN} OUTPUT index.html)
        add_dependencies(benchmark.${category} benchmark.${category}.${case})
    endif()
endmacro()


set(CMAKE_CXX_STANDARD 20)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-ftemplate-depth=5000 HAS_FTEMPLATE_DEPTH)
if(HAS_FTEMPLATE_DEPTH)
    add_compile_options(-ftemplate-depth=5000)
endif()

file(GLOB benchmark_dirs
    LIST_DIRECTORIES true
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "type/*"
)

foreach(benchmark_dir ${benchmark_dirs})
    add_subdirectory(${benchmark_dir})
endforeach()
