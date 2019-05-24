cmake_minimum_required(VERSION 3.8.2)

project(ctodo
        VERSION 0.0.1
        DESCRIPTION "Fast todo.txt implementation in C++"
        HOMEPAGE_URL "github.com/comfortablynick/ctodo"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Options: Things you can set via commandline options to cmake (e.g.
# -DENABLE_LTO=[ON|OFF])
option(ENABLE_WARNINGS_SETTINGS
       "Allow target_set_warnings to add flags and defines.
       Set this to OFF if you want to provide your own warning parameters." ON)

option(ENABLE_IWYU "Check header includes with include-what-you-use" ON)

option(ENABLE_LTO "Enable link time optimization" ON)

option(ENABLE_DOCTESTS "Include tests in the library.
  Setting this to OFF will remove all doctest related code.
Tests in tests/*.cpp will still be enabled." OFF)

# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(CTest)
include(Doctest)
include(Documentation)
include(LTO)
include(Warnings)
include(Misc)

# Check for LTO support.
find_lto(CXX)

# -----------------------------------------------------------------------------
# Locate files (change as needed).
# -----------------------------------------------------------------------------
set(TESTFILES # All .cpp files in tests/
    tests/main.cpp)

find_package(fmt)
find_package(loguru)
# find_package(args) find_package(cxxopts)

# -----------------------------------------------------------------------------
# Generate config.h
# -----------------------------------------------------------------------------
include(Configure)

# -----------------------------------------------------------------------------
# Build! (Change as needed)
# -----------------------------------------------------------------------------

add_executable(ctodo src/common.cc src/main.cc) # Name of exec. and location of
                                                # file.
target_include_directories(ctodo PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ctodo PUBLIC fmt loguru stdc++fs)
set(IWYU_TARGETS ctodo)

target_set_warnings(ctodo
                    ENABLE
                    ALL
                    AS_ERROR
                    ALL
                    DISABLE
                    Annoying) # Set warnings (if needed).
target_enable_lto(ctodo optimized) # enable link-time-optimization if available
                                   # for non-debug configurations

# Set the properties you require, e.g. what C++ standard to use. Here applied to
# library and main (change as needed).
set_target_properties(ctodo
                      PROPERTIES CXX_STANDARD
                                 17
                                 CXX_STANDARD_REQUIRED
                                 YES
                                 CXX_EXTENSIONS
                                 NO)
include(CheckIWYU)

install(TARGETS ctodo DESTINATION $ENV{HOME}/bin)
