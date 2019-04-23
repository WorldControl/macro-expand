########################################
# SOURCES
########################################

file(GLOB_RECURSE TIDY_UTILS_HRDS ${CMAKE_SOURCE_DIR}/include/misra-tidy/common/*.h*)
file(GLOB_RECURSE TIDY_UTILS_SRCS *.cpp)

set(TIDY_UTILS_SOURCES
  ${TIDY_UTILS_SRCS}
  ${TIDY_UTILS_HRDS}
)

########################################
# TARGET
########################################

add_library(tidy-utils-library STATIC ${TIDY_UTILS_SOURCES})
