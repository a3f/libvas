cmake_minimum_required(VERSION 2.8.0)
project(vas C)
set(PROJECT_VERSION 0.731)

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckTypeSize)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckCSourceRuns)
INCLUDE (CheckSymbolExists)
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#for whatever reason this doesn't work:
#CHECK_INCLUDE_FILES(ptrace.h HAVE_SYS_PTRACE_H)
CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
       #include <sys/ptrace.h>
      int main(void) { ptrace(0, 0, 0, 0); return 0; }"
      HAVE_SYS_PTRACE_H)
CHECK_INCLUDE_FILES(mach/mach.h HAVE_MACH_H)
SET(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
CHECK_SYMBOL_EXISTS(process_vm_readv sys/uio.h HAVE_PROCESS_VM_READV)
CHECK_SYMBOL_EXISTS(__GLIBC__ features.h HAVE_GLIBC)
SET(CMAKE_REQUIRED_DEFINITIONS)
CHECK_INCLUDE_FILES(windows.h HAVE_WINDOWS_H)
IF(HAVE_SYS_PTRACE_H)
    SET(CMAKE_EXTRA_INCLUDE_FILES sys/ptrace.h)
    CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
        #include <sys/ptrace.h>
        int main(void) { ptrace(PTRACE_ATTACH, 0, 0, 0); return 0; }"
        HAVE_PTRACE_ATTACH)
    CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
        #include <sys/ptrace.h>
        int main(void) { ptrace(PT_ATTACHEXC, 0, 0, 0); return 0; }"
        HAVE_PT_ATTACHEXC)
    CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
        #include <sys/ptrace.h>
        int main(void) { ptrace(PTRACE_PEEKDATA, 0, 0, 0); return 0; }"
        HAVE_PTRACE_PEEKDATA)
    CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
        #include <sys/ptrace.h>
        int main(void) { ptrace(PT_READ_D, 0, 0, 0); return 0; }"
        HAVE_PT_READ_D)
    CHECK_C_SOURCE_COMPILES("#include <sys/types.h>
        #include <sys/ptrace.h>
        int main(void) { ptrace(PT_IO, 0, 0, 0); return 0; }"
        HAVE_PT_IO)
    CHECK_TYPE_SIZE("ptrace(0, 0, 0, 0)" SIZEOF_PTRACE_RET)
    MESSAGE(STATUS "Return type of ptrace is ${SIZEOF_PTRACE_RET} bytes wide")
    SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDIF(HAVE_SYS_PTRACE_H)

include(gmxTestLargeFiles)
gmx_test_large_files(GMX_LARGEFILES)

CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(int   SIZEOF_INT)
CHECK_TYPE_SIZE(long  SIZEOF_LONG)
CHECK_TYPE_SIZE(off_t SIZEOF_OFF_T)
CHECK_TYPE_SIZE(pid_t SIZEOF_PID_T)

IF (SIZEOF_OFF_T)
SET(VAS_HAVE_OFF_T 1)
ENDIF()
IF (SIZEOF_PID_T)
SET(VAS_HAVE_PID_T 1)
ENDIF()

CHECK_INCLUDE_FILES(libpid.h HAVE_LIBPID_H)
IF (HAVE_LIBPID_H)
    CHECK_SYMBOL_EXISTS(pid_self libpid.h HAVE_PID_SELF)
ENDIF()
IF (NOT HAVE_PID_SELF)
    SET(CMAKE_REQUIRED_INCLUDES sys/types.h)
    CHECK_SYMBOL_EXISTS(getpid unistd.h HAVE_GETPID)
    SET(CMAKE_REQUIRED_INCLUDES)
    IF (HAVE_GETPID)
        SET(pid_self getpid)
    ELSE()
        CHECK_SYMBOL_EXISTS(_getpid process.h HAVE__GETPID)
        IF (HAVE__GETPID)
            SET(pid_self _getpid)
        ELSE()
            MESSAGE(WARNING No function for determining PID found! Some backends/tests will be unusable)
        ENDIF()
    ENDIF()
ENDIF()
CHECK_SYMBOL_EXISTS(fork unistd.h HAVE_FORK)
CHECK_SYMBOL_EXISTS(CreateProcessW windows.h HAVE_CREATEPROCESS)

CHECK_INCLUDE_FILES(sys/mman.h VAS_HAVE_MMAN_H)
IF (VAS_HAVE_MMAN_H)
    CHECK_SYMBOL_EXISTS(MAP_ANONYMOUS sys/mman.h HAVE_MAP_ANONYMOUS)
    check_symbol_exists(O_SYNC "fcntl.h" HAVE_O_SYNC)
    IF (NOT HAVE_MAP_ANONYMOUS)
        CHECK_SYMBOL_EXISTS(MAP_ANON sys/mman.h HAVE_MAP_ANON)
    ENDIF()
    CHECK_SYMBOL_EXISTS(shm_open sys/mman.h HAVE_SHM_OPEN)
    CHECK_SYMBOL_EXISTS(shm_mkstemp sys/mman.h HAVE_SHM_MKSTEMP)
    IF (NOT HAVE_SHM_OPEN)
        SET(CMAKE_REQUIRED_LIBRARIES rt)
        MESSAGE(STATUS "Trying with -lrt")
        CHECK_SYMBOL_EXISTS(shm_open sys/mman.h HAVE_SHM_OPEN_WITH_LIBRT)
        CHECK_SYMBOL_EXISTS(shm_mkstemp sys/mman.h HAVE_SHM_MKSTEMP_WITH_LIBRT)
        SET(CMAKE_REQUIRED_LIBRARIES)
        IF(HAVE_SHM_OPEN_WITH_LIBRT)
            SET(HAVE_SHM_OPEN 1)
        ENDIF()
        IF(HAVE_SHM_MKSTEMP_WITH_LIBRT)
            SET(HAVE_SHM_MKSTEMP 1)
        ENDIF()
    ENDIF()
ENDIF(VAS_HAVE_MMAN_H)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
IF(NOT HAVE_WINDOWS_H)
    set(CMAKE_MACOSX_RPATH ON)
ENDIF()
include_directories(include)
include_directories(.)

file(GLOB SOURCES info.c)

STRING(REGEX MATCH "Linux" LINUX ${CMAKE_SYSTEM_NAME})
STRING(REGEX MATCH "BSD" BSD ${CMAKE_SYSTEM_NAME})

STRING(TOLOWER "${BACKEND}" BACKEND)

# IF(EXISTS finds /proc/self/mem but not /proc/1/mem for whatever reason
# FreeBSD has no /proc/1/mem, but has /proc/anyother_pid/mem

MESSAGE(STATUS "Checking for /proc/$$/as")
CHECK_C_SOURCE_RUNS("#include <sys/types.h>
    #include <unistd.h>
    #include <stdio.h>
    char filename[32];
    int main(void) {
        if (sprintf(filename, \"/proc/%ld/as\", (long)getpid()) < 0) return 1;
        return access( filename, F_OK );}"
        HAVE_PROCFS_PID_AS)

IF (HAVE_PROCFS_PID_AS)
    SET(PROCFS_MEM_OR_AS "as")
ENDIF()

MESSAGE(STATUS "Checking for /proc/$$/mem")
CHECK_C_SOURCE_RUNS("#include <sys/types.h>
    #include <unistd.h>
    #include <stdio.h>
    char filename[32];
    int main(void) {
        if (sprintf(filename, \"/proc/%ld/mem\", (long)getpid()) < 0) return 1;
        return access( filename, F_OK );}"
        HAVE_PROCFS_PID_MEM)

IF (HAVE_PROCFS_PID_MEM)
    SET(PROCFS_MEM_OR_AS "mem")
ENDIF()

IF (PROCFS_MEM_OR_AS)
    # Linux
    CHECK_C_SOURCE_RUNS("#include <sys/types.h>
        #include <unistd.h>
        int main(void) {
            return access( \"/proc/self/${PROCFS_MEM_OR_AS}\", F_OK );}"
            HAVE_PROCFS_SELF_SYMLINK)
    # FreeBSD
    CHECK_C_SOURCE_RUNS("#include <sys/types.h>
        #include <unistd.h>
        int main(void) {
            return access( \"/proc/curproc/${PROCFS_MEM_OR_AS}\", F_OK );}"
            HAVE_PROCFS_CURPROC_SYMLINK)
ENDIF()


IF("${BACKEND}" STREQUAL "")
    IF(WIN32)
        SET(BACKEND win32)
    ELSEIF(HAVE_MACH_H) # macOS and GNU Hurd
        SET(BACKEND mach)
    ELSEIF(HAVE_PROCESS_VM_READV) # Linux 3.2 & glibc 2.15
        SET(BACKEND process_vm)
    ELSEIF(HAVE_PROCFS_PID_MEM # Linux and more
        OR HAVE_PROCFS_PID_AS) # SunOS/Solaris
        SET(BACKEND procfs)
    ELSEIF(HAVE_SYS_PTRACE_H AND NOT APPLE) # BSD without procfs mounted, what else?
        SET(BACKEND ptrace)                 # macOS always reports EBUSY instead of proper ESYS

    ELSE() 
        SET(BACKEND memcpy)
    ENDIF()
ENDIF()

IF("${BACKEND}" STREQUAL win32)
    MESSAGE("lib${PROJECT_NAME} will use WinAPI")
    SET(VAS_USES_WINAPI 1)
    file(GLOB SOURCES ${SOURCES} win32/*.c poll.c dup.c)

ELSEIF("${BACKEND}" STREQUAL mach)
    MESSAGE("lib${PROJECT_NAME} will use Mach VM API")
    SET(VAS_USES_MACH 1)
    file(GLOB SOURCES ${SOURCES} mach/*.c posix/fmap.c dup.c)

ELSEIF("${BACKEND}" STREQUAL process_vm)
    file(GLOB SOURCES ${SOURCES} process_vm/*.c posix/*.c poll.c dup.c)
    IF (NOT (HAVE_PID_SELF OR pid_self))
        MESSAGE(FATAL_ERROR "Backend requires a function to retrieve own pid, but none found.")
    ENDIF()
    SET(VAS_USES_PROCESS_VM 1)
    MESSAGE("lib${PROJECT_NAME} will use process_vm_{readv,writev}")

ELSEIF("${BACKEND}" STREQUAL procfs)
    MESSAGE("lib${PROJECT_NAME} will use /proc/$pid/${PROCFS_MEM_OR_AS}")
    SET(VAS_USES_PROCFS 1)
    IF (NOT (HAVE_PID_SELF OR pid_self))
        MESSAGE(FATAL_ERROR "Backend requires a function to retrieve own pid, but none found.")
    ENDIF()
    INCLUDE (CheckFunctionExists)
        CHECK_FUNCTION_EXISTS(pwrite HAVE_PWRITE)
        CHECK_FUNCTION_EXISTS(pread HAVE_PREAD)
        IF (NOT HAVE_PWRITE OR NOT HAVE_PREAD)
            MESSAGE(WARNING "using own implementation for pread/pwrite(2)")
        ENDIF()
    configure_file(config.h.in config.h)
    file(GLOB SOURCES ${SOURCES} procfs/*.c posix/*.c poll.c dup.c)
    #add_definitions(-DHAS_WEAK_ALIAS)

ELSEIF("${BACKEND}" STREQUAL ptrace)
    IF(BSD)
        MESSAGE(STATUS "procfs not mounted on this BSD. You could try to mount -t procfs proc /proc")
    ENDIF()
    MESSAGE("lib${PROJECT_NAME} will use ptrace(2) API")
    SET(VAS_USES_PTRACE 1)
    IF (NOT (HAVE_PID_SELF OR pid_self))
        MESSAGE(FATAL_ERROR "Backend requires a function to retrieve own pid, but none found.")
    ENDIF()
    SET(CANT_SELFTEST 1)
    file(GLOB SOURCES ${SOURCES} ptrace/*.c posix/*.c poll.c dup.c)

ELSEIF("${BACKEND}" STREQUAL memcpy)
    MESSAGE("lib${PROJECT_NAME} will use memcpy")
    SET(VAS_USES_MEMCPY 1)
    MESSAGE(WARNING "It looks like we don't support this system yet,")
    MESSAGE(WARNING "Falling back to trivial (own address space only) implementation.")
    file(GLOB SOURCES ${SOURCES} *.c)

ELSE()
    MESSAGE(FATAL_ERROR "${BACKEND} is not a supported backend. Valid values are
      win32, mach, procfs, process_vm, ptrace, memcpy
      or just leave it empty")

ENDIF()



configure_file(vas.h.in vas.h)
configure_file(vas-types.h.in vas-types.h)
configure_file(config.h.in config.h)
configure_file(libvas.pc.in libvas.pc @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/CFLAGS.cmake)

IF(HAVE_SHM_OPEN_WITH_LIBRT) # TODO also check if shm_open is used at all
    link_libraries(rt)
ENDIF()
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Tests

SET(AUTO_TESTS onself cow ringbuf)

IF(HAVE_FORK)
    LIST(APPEND AUTO_TESTS onchild_fork)
ELSEIF(HAVE_CREATEPROCESS)
ENDIF(HAVE_FORK)

enable_testing()

IF(VAS_USES_MEMCPY)
    LIST(REMOVE_ITEM AUTO_TESTS ringbuf)
    LIST(REMOVE_ITEM AUTO_TESTS onchild_fork)
ENDIF()


foreach(T ${AUTO_TESTS})
    file(GLOB ${T}_src test/${T}.c)
    add_executable(${T} ${${T}_src})
    target_link_libraries(${T} ${PROJECT_NAME})
    set_target_properties(${T} PROPERTIES OUTPUT_NAME "${T}")
endforeach(T)

IF(CANT_SELFTEST)
    LIST(REMOVE_ITEM AUTO_TESTS onself cow)
ENDIF()

FOREACH(T ${AUTO_TESTS}) 
    add_test(${T} ${CMAKE_CURRENT_BINARY_DIR}/${T})
ENDFOREACH()


SET(MANUAL_TESTS readlong)

IF(WIN32)
    LIST(APPEND MANUAL_TESTS tibia-healer)
ENDIF()

foreach(T ${MANUAL_TESTS})
    file(GLOB ${T}_src test/manual/${T}.c)
    add_executable(${T} ${${T}_src})
    target_link_libraries(${T} ${PROJECT_NAME})
    set_target_properties(${T} PROPERTIES OUTPUT_NAME "${T}")
endforeach(T)


# Install pkg-config pc
install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc DESTINATION share/pkgconfig)

# Install library
install (TARGETS ${PROJECT_NAME} DESTINATION lib)

# Install library headers
install(FILES ${CMAKE_BINARY_DIR}/vas.h ${CMAKE_BINARY_DIR}/vas-types.h DESTINATION include)


