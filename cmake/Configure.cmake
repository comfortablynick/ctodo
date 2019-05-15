# -- Have CMake parse the config file, generating the config header, with
# correct definitions. Here only used to make version number available to the
# source code. Include "config.h" in source code

# Preprocessing define pkg vars in config.h
set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(PACKAGE_TARNAME ${PROJECT_NAME})
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_URL ${PROJECT_HOMEPAGE_URL})
set(PACKAGE_BUGREPORT "comfortablynick@gmail.com")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

# Generate config.h
message(STATUS "Running configure")
configure_file("${PROJECT_SOURCE_DIR}/include/config.h.in"
               "${PROJECT_SOURCE_DIR}/include/config.h" @ONLY)
message(STATUS "Running configure - done")
