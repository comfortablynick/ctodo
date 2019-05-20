# include-what-you-use
if(ENABLE_IWYU)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(
      STATUS "Could not find the program include-what-you-use; skipping...")
  else()
    foreach(TGT ${IWYU_TARGETS})
      message(STATUS "Enabling include-what-you-use for ${TGT}")
      set_property(TARGET ${TGT} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
      message(STATUS "Enabling include-what-you-use for ${TGT} - done")
    endforeach()
  endif()
endif()
