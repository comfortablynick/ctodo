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
  # option(build_static_lib "Build easyloggingpp as a static library" ON)
  # add_subdirectory(${easyloggingpp_SOURCE_DIR} ${easyloggingpp_BINARY_DIR})
  include_directories("${easyloggingpp_SOURCE_DIR}/src")
  add_library(easyloggingpp STATIC
              ${easyloggingpp_SOURCE_DIR}/src/easylogging++.cc)
  # target_link_options(easyloggingpp PUBLIC -pthread -lpthread -ldl)
endif()
