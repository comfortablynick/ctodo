set(pkg dimcli)
option(BUILD_TESTING "Enable test generation for ctest" OFF)
include(FetchContent)
FetchContent_Declare(${pkg}
                     GIT_REPOSITORY https://github.com/gknowles/dimcli.git
                     # GIT_TAG 73aa158d38a35e9b4d0cfeb6dbf8fd8d73ddfa93)
                     GIT_TAG master)
FetchContent_GetProperties(${pkg})
if(NOT ${pkg}_POPULATED)
  FetchContent_Populate(${pkg})
  message(
    STATUS
      "${BoldCyan}PACKAGE ${pkg} source dir: ${${pkg}_SOURCE_DIR}${ColorReset}")
  message(
    STATUS
      "${BoldCyan}PACKAGE ${pkg} binary dir: ${${pkg}_BINARY_DIR}${ColorReset}")
  set(CMAKE_INSTALL_PREFIX ${${pkg}_SOURCE_DIR})
  add_subdirectory(${${pkg}_SOURCE_DIR})
  include_directories("${${pkg}_SOURCE_DIR}/libs")
endif()
