########################################
# SOURCES
########################################

file(GLOB_RECURSE MACRO_EXPAND_HRDS ${CMAKE_SOURCE_DIR}/include/misra-tidy/macro-expand/*.h*)
file(GLOB_RECURSE MACRO_EXPAND_SRCS *.cpp)

set(MACRO_EXPAND_SOURCES
  ${MACRO_EXPAND_SRCS}
  ${MACRO_EXPAND_HRDS}
)

########################################
# TARGET
########################################

add_library(macro-expand-library STATIC ${MACRO_EXPAND_SOURCES})
