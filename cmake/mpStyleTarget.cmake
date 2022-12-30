function(mp_create_style_target target_name)
  set(style_commands_file "${PROJECT_BINARY_DIR}/cmake/mpStyleCommands.cmake")
  set(style_commands_content
    [=[
    set(clang_format_inputs)

    foreach(subdir "benchmark" "example" "include")
      file(GLOB_RECURSE subdir_sources
        LIST_DIRECTORIES false
        RELATIVE "@PROJECT_SOURCE_DIR@"
        "@PROJECT_SOURCE_DIR@/${subdir}/*.[ch]pp")

      list(APPEND clang_format_inputs "${subdir_sources}")
    endforeach()

    execute_process(
      ECHO_OUTPUT_VARIABLE
      ECHO_ERROR_VARIABLE
      WORKING_DIRECTORY "@PROJECT_SOURCE_DIR@"
      COMMAND echo done
      COMMAND clang-format --verbose -style=file:.clang-format -i ${clang_format_inputs})
    ]=])

  string(CONFIGURE "${style_commands_content}" style_commands_content @ONLY)

  file(GENERATE
    OUTPUT "${style_commands_file}"
    CONTENT "${style_commands_content}")

  add_custom_target(${target_name})
  add_custom_command(
    TARGET ${target_name}
    COMMAND "${CMAKE_COMMAND}" -P "${style_commands_file}")
endfunction()
