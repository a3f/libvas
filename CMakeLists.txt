cmake_minimum_required(VERSION 2.6)
project(vas C)
set(PROJECT_VERSION 0.41)
set(CMAKE_BUILD_TYPE Debug)

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES(pid.h VAS_HAVE_PID_H)
CHECK_INCLUDE_FILES(mach/mach.h HAVE_MACH_H)




configure_file(vas.h.in vas.h)
configure_file(config.h.in config.h)
configure_file(libvas.pc.in libvas.pc @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/CFLAGS.cmake)

include_directories(include)
include_directories(.)

file(GLOB SOURCES info.c)

STRING(REGEX MATCH "Linux" LINUX ${CMAKE_SYSTEM_NAME})
STRING(REGEX MATCH "BSD" BSD ${CMAKE_SYSTEM_NAME})

IF(WIN32)
    MESSAGE(STATUS "lib${PROJECT_NAME} will use WinAPI")
    SET(VAS_USES_WINAPI 1)
    file(GLOB SOURCES ${SOURCES} win32/*.c poll.c)

ELSEIF(HAVE_MACH_H) # macOS and GNU Hurd
    MESSAGE(STATUS "lib${PROJECT_NAME} will use Mach VM API")
    SET(VAS_USES_MACH 1)
    set(CMAKE_MACOSX_RPATH 1) 
    file(GLOB SOURCES ${SOURCES} mach/*.c)

ELSEIF(EXISTS "/proc/self/mem") # Linux and more
    MESSAGE(STATUS "lib${PROJECT_NAME} will use procfs")
    SET(VAS_USES_PROCFS 1)
    # Are there procfs' which have no /proc/self symlink?
    INCLUDE (CheckFunctionExists)
        CHECK_FUNCTION_EXISTS(pwrite HAVE_PWRITE )
        CHECK_FUNCTION_EXISTS(pread HAVE_PREAD)
        IF (NOT HAVE_PWRITE OR NOT HAVE_PREAD)
            MESSAGE(WARNING "using own implementation for pread/pwrite(2)")
        ENDIF()
    configure_file(config.h.in config.h)
    file(GLOB SOURCES ${SOURCES} procfs/*.c poll.c)
    #add_definitions(-DHAS_WEAK_ALIAS)

ELSEIF(BSD)
    MESSAGE(STATUS "lib${PROJECT_NAME} will use memcpy")
    SET(VAS_USES_MEMCPY 1)
    MESSAGE(WARNING "Please mount procfs and try again")
    MESSAGE(WARNING "Falling back to trivial (own address space only) implementation.")
    file(GLOB SOURCES ${SOURCES} *.c)

ELSE()
    MESSAGE(STATUS "lib${PROJECT_NAME} will use memcpy")
    SET(VAS_USES_MEMCPY 1)
    MESSAGE(WARNING "It looks like we don't support this system yet,")
    MESSAGE(WARNING "Falling back to trivial (own address space only) implementation.")
    file(GLOB SOURCES ${SOURCES} *.c)

ENDIF(WIN32)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Install pkg-config pc
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc DESTINATION share/pkgconfig)

# Install library
install (TARGETS ${PROJECT_NAME} DESTINATION lib)

# Install library headers
install(FILES ${CMAKE_BINARY_DIR}/vas.h DESTINATION include)

