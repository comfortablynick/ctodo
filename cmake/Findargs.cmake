# Findfmt.cmake (Find shared library first if installed)
set(pkg args)
find_library(FOUND_LIBRARY ${pkg})
if(FOUND_LIBRARY)
  message(STATUS "PACKAGE ${pkg} library installed: ${FOUND_LIBRARY}")
else()
  include(FetchContent)
  FetchContent_Declare(${pkg}
                       GIT_REPOSITORY https://github.com/Taywee/args.git
                       GIT_TAG 623e52cb128cfc572ad4e79de8d5f8861d13d017)
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
    target_sources(${pkg} INTERFACE ${${pkg}_SOURCE_DIR}/args.hxx)
  endif()
endif()
