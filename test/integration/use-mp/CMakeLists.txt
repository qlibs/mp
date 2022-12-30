cmake_minimum_required(VERSION 3.23)
project(use-mp)
enable_testing()

foreach(expected_var "mp_ROOT" "consumption_type")
  if(NOT DEFINED ${expected_var})
    message(FATAL_ERROR "'${expected_var}' must be provided to the ${PROJECT_NAME} test project.")
  endif()
endforeach()

if(consumption_type STREQUAL "find")
  find_package(mp CONFIG REQUIRED)
else()
  add_subdirectory(
    "${PROJECT_SOURCE_DIR}/../../../"
    "${PROJECT_BINARY_DIR}/build-mp" SYSTEM)
endif()

add_executable(use-mp main.cpp)
target_link_libraries(use-mp PRIVATE Boost::mp)
add_test(NAME use-mp COMMAND use-mp)
