macro(_include_directories TARGET DIR SYSTEM)
  if (CMAKE_VERSION VERSION_LESS 2.8.12 OR "${SYSTEM}" EQUAL "TRUE")
    target_include_directories("${TARGET}" PUBLIC "${DIR}")
    message(STATUS "including ${DIR} for ${TARGET} as NON-SYSTEM")
  else (CMAKE_VERSION VERSION_LESS 2.8.12 OR "${SYSTEM}" EQUAL "TRUE")
    target_include_directories("${TARGET}" SYSTEM PUBLIC "${DIR}")
    message(STATUS "including ${DIR} for ${TARGET} as SYSTEM")
  endif (CMAKE_VERSION VERSION_LESS 2.8.12 OR "${SYSTEM}" EQUAL "TRUE")
endmacro(_include_directories TARGET DIR SYSTEM)
