cmake_minimum_required(VERSION 2.8)

project(OpenTechBFG)

message(STATUS CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE})

## Compiler specific options
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  # GCC includes MinGW
  include(${CMAKE_CURRENT_SOURCE_DIR}/gcc.cmake)
elseif(MSVC)
  include(${CMAKE_CURRENT_SOURCE_DIR}/msvc.cmake)
endif() # MSVC

#if(STANDALONE)
#	add_definitions(-DSTANDALONE)
#endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/idlib.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/tools.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/common.cmake)

## LIBRARIES

include(${CMAKE_CURRENT_SOURCE_DIR}/libraries.cmake)

## TARGETS

if(NOT UNIX) # Windows (MSVC and MinGW)
  include(${CMAKE_CURRENT_SOURCE_DIR}/windows.cmake)
else() # UNIX
  include(${CMAKE_CURRENT_SOURCE_DIR}/unix.cmake)
endif()
