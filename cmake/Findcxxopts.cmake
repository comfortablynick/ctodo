set(pkg cxxopts)
# option(CXXOPTS_BUILD_EXAMPLES "Build examples" OFF) option(CXXOPTS_BUILD_TESTS
# "Build tests" OFF)
find_library(FOUND_LIBRARY ${pkg})
if(FOUND_LIBRARY)
  message(STATUS "PACKAGE ${pkg} library installed: ${FOUND_LIBRARY}")
else()
  include(FetchContent)
  FetchContent_Declare(${pkg}
                       GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
                       GIT_TAG e34676f73e49eeff30cb101f1c5ba8806fbe6773)
  FetchContent_GetProperties(${pkg})
  if(NOT ${pkg}_POPULATED)
    FetchContent_Populate(${pkg})
    message(
      STATUS
        "${BoldCyan}PACKAGE ${pkg} source dir: ${${pkg}_SOURCE_DIR}${ColorReset}"
      )
    message(
      STATUS
        "${BoldCyan}PACKAGE ${pkg} binary dir: ${${pkg}_BINARY_DIR}${ColorReset}"
      )
    include_directories("${${pkg}_SOURCE_DIR}")
    add_library(${pkg} INTERFACE)
    target_sources(${pkg} INTERFACE ${${pkg}_SOURCE_DIR}/include/cxxopts.hpp)
  endif()
endif()
