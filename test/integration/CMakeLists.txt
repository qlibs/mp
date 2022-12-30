set(mp_ROOT "${CMAKE_CURRENT_BINARY_DIR}/install")

function(create_use_mp_test consumption_type)
  if(NOT consumption_type MATCHES "find|add")
    message(FATAL_ERROR "Only 'find' or 'add' may be provided.")
  endif()

  set(test_name ${consumption_type}-use-mp)
  add_test(
    NAME ${test_name}
    COMMAND
    "${CMAKE_CTEST_COMMAND}"
    --verbose
    --output-on-failure
    --build-project "use-mp" # helps msvc if --build-target ever provided
    --build-generator "${CMAKE_GENERATOR}"
    --build-config $<CONFIG>
    --build-and-test
      "${CMAKE_CURRENT_SOURCE_DIR}/use-mp"
      "${CMAKE_CURRENT_BINARY_DIR}/use-mp"
    --build-options
      "-Dconsumption_type=${consumption_type}"
      "-Dmp_ROOT:PATH=${mp_ROOT}"
      "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
      "-DCMAKE_CXX_EXTENSIONS=OFF"
    --test-command "${CMAKE_CTEST_COMMAND}" --verbose --output-on-failure)

  set_tests_properties(${test_name}
    PROPERTIES
      RESOURCE_LOCK "use-mp-lock"
      FIXTURES_REQUIRED install-mp-fixture)
endfunction()

add_test(
  NAME install-mp
  COMMAND
  "${CMAKE_COMMAND}"
  --install "${PROJECT_BINARY_DIR}"
  --prefix "${mp_ROOT}"
  --config $<CONFIG>
  --verbose)

set_tests_properties(install-mp PROPERTIES
  FIXTURES_SETUP install-mp-fixture)

create_use_mp_test("find")
create_use_mp_test("add")
