###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 3.5)
project(misra-tidy)

###########################################################
## DOCUMENTATION
###########################################################

# Add a target to generate documentation with Doxygen.
find_package(Doxygen)
if(DOXYGEN_FOUND)
  # Replace all variables of the form @<var>@ inside the Doxyfile
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
  )
  add_custom_target(
    docs
    ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif()

###########################################################
## DEPENDENCIES
###########################################################

set(CMAKE_MODULE_PATH
   ${CMAKE_MODULE_PATH}
   "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(CLANG_LIBS
  clangAST
  clangASTMatchers
  clangAnalysis
  clangBasic
  clangDriver
  clangEdit
  clangFrontend
  clangFrontendTool
  clangLex
  clangParse
  clangSema
  clangEdit
  clangRewrite
  clangRewriteFrontend
  clangSerialization
  clangTooling
  clangToolingCore
)
set(MW_CLANG_INCLUDE_DIRS 
L:/mathworks-hub/3rdparty/R2018a/2932568/win64/Clang/Release/include
)
set(MW_CLANG_LIBRARY_DIRS
L:/mathworks-hub/3rdparty/R2018a/2932568/win64/Clang/Release/lib
)

if (NOT CMAKE_HOST_APPLE)
    if (MSVC)
      set(listVar "")
      foreach(f ${CLANG_LIBS})
      list(APPEND listVar "${f}.lib")
      endforeach(f)
      set(CLANG_LIBS ${listVar})
      set(CLANG_LIBS ${CLANG_LIBS})
    else()
      set(CLANG_LIBS -Wl,--start-group ${CLANG_LIBS} -Wl,--end-group)
    endif()
endif()

find_package(LLVM REQUIRED)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
include_directories(SYSTEM ${MW_CLANG_INCLUDE_DIRS})

# Third Party
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

# Also contains clang libraries
link_directories(${LLVM_LIBRARY_DIRS})
link_directories(${MW_CLANG_LIBRARY_DIRS})
set(CMAKE_EXE_LINKER_FLAGS ${LLVM_LD_FLAGS_STRING})

###########################################################
## COMPILER FLAGS
###########################################################

set(CXX_STANDARD_REQUIRED ON)

list(REMOVE_ITEM LLVM_CXX_FLAGS
                 ${LLVM_INCLUDE_DIRS} # Already included
                 "-DNDEBUG"
                 "-std=c++11"         # Want our own standard
                 "-std=c++1y"
                 "-stdlib=libc++"
                 "-fno-rtti-DLLVM_BUILD_GLOBAL_ISEL" # Weird bugs on Debian
                 )

if (NOT ${CMAKE_CXX_COMPILER} MATCHES "clang.*")
  # These seem to be clang-specific
  list(REMOVE_ITEM LLVM_CXX_FLAGS
                   "-Wcovered-switch-default"
                   "-Wstring-conversion")
endif()

set(EXTRA_FLAGS
    -fdata-sections
    -std=c++14
    -fno-rtti
    -DLLVM_BUILD_GLOBAL_ISEL
    -DJSON_NOEXCEPTION)
set(ALL_FLAGS ${WARNINGS} ${LLVM_CXX_FLAGS} ${EXTRA_FLAGS})

add_compile_options(${ALL_FLAGS})

###########################################################
## INCLUDES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###########################################################
## SOURCES
###########################################################

add_subdirectory(source)

###########################################################
## TARGETS
###########################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
   ${CMAKE_BINARY_DIR}/bin/${MACRO_EXPAND_OS_NAME})
message(STATUS "Binaries will land in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

file(GLOB_RECURSE MACRO_EXPAND_EXE_SRCS source/macro-expand-exe/*.cpp)
file(GLOB_RECURSE MACRO_EXPAND_EXE_HDRS source/macro-expand-exe/*.hpp)

add_executable(macro-expand ${MACRO_EXPAND_EXE_SRCS} ${MACRO_EXPAND_EXE_HDRS})
if (MSVC)
  set(listVar "")
  foreach(f ${LLVM_LIBS})
  string(REPLACE "-l" "" STRIPPED_LIB ${f})
  list(APPEND listVar "${STRIPPED_LIB}.lib")
  endforeach(f)
  set(LLVM_LIBS ${listVar})
  string(REPLACE "/MDd" "/MD" NewDebugFlags ${CMAKE_C_FLAGS_DEBUG})
  message(STATUS "Original CMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_DEBUG ${NewDebugFlags})
  message(STATUS "New CMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}")
endif()
target_link_libraries(macro-expand
                      macro-expand-library
                      tidy-utils-library
                      ${CLANG_LIBS}
                      ${LLVM_LIBS})

###########################################################
## DOCKER
###########################################################

find_program(DOCKER_COMPOSE NAMES docker-compose)
if (DOCKER_COMPOSE)
  message(STATUS "Found docker-compose at ${DOCKER_COMPOSE}")
  add_custom_target(
    docker
    COMMAND docker-compose up
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    COMMENT "Building macro-expand with docker" VERBATIM
  )
  message(STATUS "Enabled 'docker' target for building binaries")
endif()

###########################################################
## TOOLS <3
###########################################################

# IWYU
option(MACRO_EXPAND_IWYU OFF)

if(${MACRO_EXPAND_IWYU})
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(iwyu_path)
    message(STATUS "Found include-what-you-use, integrating with target")
    set_property(TARGET macro-expand
                 PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
  else()
    message(WARNING "Could not find include-what-you-use, skipping")
  endif()
endif()
