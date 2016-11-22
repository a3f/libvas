cmake_minimum_required(VERSION 2.6)
project(vas)
set(PROJECT_VERSION 0.1)
set(CMAKE_BUILD_TYPE Debug)

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES(pid.h VAS_HAVE_PID_H)

configure_file(vas.h.in vas.h)
configure_file(libvas.pc.in libvas.pc @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUC)
  # Update if necessary
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic -std=c89")
endif()

include_directories(include)
include_directories(.)
IF(WIN32)
    file(GLOB SOURCES win32/*.c poll.c)
ELSEIF(APPLE)
    set(CMAKE_MACOSX_RPATH 1) 
    file(GLOB SOURCES xnu/*.c)
    #add_definitions(-DHAS_WEAK_ALIAS)
ELSEIF(LINUX)
    file(GLOB SOURCES linux/*.c poll.c)
    #add_definitions(-DHAS_WEAK_ALIAS)
ELSE()
    file(GLOB SOURCES *.c)
ENDIF(WIN32)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Install pkg-config pc
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc DESTINATION share/pkgconfig)

# Install library
install (TARGETS ${PROJECT_NAME}
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)

# Install library headers
install(FILES ${CMAKE_BINARY_DIR}/vas.h DESTINATION include)

