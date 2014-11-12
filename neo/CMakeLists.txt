cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

project(OpenTechBFG)

message(STATUS CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE})

## Compiler specific options
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  # GCC includes MinGW
  include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/gcc.cmake)
  # https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html#C_002b_002b-Dialect-Options
  add_definitions(-std=c++03)

  add_definitions(-Wdelete-non-virtual-dtor)
  add_definitions(-Wnon-virtual-dtor)
  add_definitions(-Wctor-dtor-privacy)

  #add_definitions(-Wold-style-cast)
  add_definitions(-Wsign-promo)
  add_definitions(-Wliteral-suffix)
  add_definitions(-Wnarrowing)
  add_definitions(-Wnoexcept)
  add_definitions(-Wreorder)
  add_definitions(-Wstrict-null-sentinel)
  add_definitions(-Woverloaded-virtual)

  #add_definitions(-fvtable-verify=std)
  # perhaps someday:
  #add_definitions(-Weffc++)
  add_definitions(-Wall)
  add_definitions(-Wno-unknown-pragmas)
  add_definitions(-Wno-unused-local-typedefs)
  add_definitions(-Wno-unused-function)
  add_definitions(-Wno-switch)
  add_definitions(-Wno-sign-compare)
  add_definitions(-Wno-invalid-offsetof)
  #add_definitions(-pedantic) 
  
  if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_definitions(-Wno-unknown-warning-option)
  endif()

elseif(MSVC)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/msvc.cmake)
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
