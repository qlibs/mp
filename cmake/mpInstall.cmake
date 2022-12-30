include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(mp_name_version "${PROJECT_NAME}-${PROJECT_VERSION}")
set(mp_include_install_dest "${CMAKE_INSTALL_INCLUDEDIR}/${mp_name_version}")
set(mp_cmake_install_dest "${CMAKE_INSTALL_DATAROOTDIR}/cmake/${mp_name_version}")
set(mp_doc_install_dest "${CMAKE_INSTALL_DATAROOTDIR}/doc/${mpm_name_version}")

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${mp_cmake_install_dest}")

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
  ARCH_INDEPENDENT)

install(
  FILES
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${mp_cmake_install_dest}"
  COMPONENT devel)

install(
  TARGETS mp
  EXPORT mpTargets
  FILE_SET mp_headers
  DESTINATION "${mp_include_install_dest}"
  COMPONENT devel
  INCLUDES DESTINATION "${mp_include_install_dest}")

install(
  EXPORT mpTargets
  NAMESPACE Boost::
  DESTINATION "${mp_cmake_install_dest}"
  COMPONENT devel)

install(
  FILES "${PROJECT_SOURCE_DIR}/LICENSE.md"
  DESTINATION "${mp_doc_install_dest}")
