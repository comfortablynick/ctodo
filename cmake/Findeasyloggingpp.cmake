# Finds easylogging++ This will define the following variables
# EASYLOGGINGPP_FOUND and the following imported target
# easyloggingpp::easyloggingpp Created by Christian von Arnim (Github @ccvca)

find_library(EASYLOGGINGPP_LIB easyloggingpp)

if(NOT "${EASYLOGGINGPP_LIB}" STREQUAL "EASYLOGGINGPP_LIB-NOTFOUND")
  message("Found easylogging library: '${EASYLOGGINGPP_LIB}'")
endif()

find_path(EASYLOGGINGPP_INCLUDE easylogging++.h)
if(NOT "${EASYLOGGINGPP_INCLUDE}" STREQUAL "EASYLOGGINGPP_INCLUDE-NOTFOUND")
  message(STATUS "Found easylogging++.h in:'${EASYLOGGINGPP_INCLUDE}' ")
  set(EASYLOGGINGPP_FOUND 1)
endif()

if(NOT EASYLOGGINGPP_FOUND)
  message(STATUS "Could not found easylogging++ locally")
  message(STATUS "Getting external package easylogging++")
  include(FetchContent)
  FetchContent_Declare(
    easyloggingpp
    GIT_REPOSITORY https://github.com/zuhd-org/easyloggingpp.git
    GIT_TAG 20fdea6e1b5ae42fb50c9179e332c22242030a38)
  FetchContent_GetProperties(easyloggingpp)
  if(NOT easyloggingpp_POPULATED)
    FetchContent_Populate(easyloggingpp)
    message(
      STATUS "PACKAGE easyloggingpp source dir: ${easyloggingpp_SOURCE_DIR}")
    message(
      STATUS "PACKAGE easyloggingpp binary dir: ${easyloggingpp_BINARY_DIR}")
    # include_directories("${easyloggingpp_SOURCE_DIR}/src")
    set(EASYLOGGINGPP_INCLUDE ${easyloggingpp_SOURCE_DIR}/src)
    set(EASYLOGGINGPP_FOUND 1)
  endif()
endif()

if(EASYLOGGINGPP_FOUND)
  add_library(easyloggingpp ${EASYLOGGINGPP_INCLUDE}/easylogging++.cc)
  target_compile_definitions(easyloggingpp
                             PUBLIC
                             ELPP_STL_LOGGING
                             ELPP_THREAD_SAFE
                             ELPP_NO_DEFAULT_LOG_FILE
                             ELPP_USE_STD_THREADING)

  # add_library(easyloggingpp::easyloggingpp IMPORTED INTERFACE)
  # target_link_libraries(easyloggingpp::easyloggingpp INTERFACE easyloggingpp)
  target_include_directories(easyloggingpp
                             INTERFACE ${EASYLOGGINGPP_INCLUDE})
  add_library(easyloggingpp::easyloggingpp ALIAS easyloggingpp)
endif()
