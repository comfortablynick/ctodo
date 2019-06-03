set(pkg cli)
# set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
option(CLI11_INSTALL
       "Install the CLI11 folder to include during install process" OFF)
option(CLI11_SINGLE_FILE "Generate a single header file" ON)
include(FetchContent)
FetchContent_Declare(${pkg}
                     GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
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
  add_subdirectory(${${pkg}_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif()
