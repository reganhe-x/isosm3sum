cmake_minimum_required(VERSION 3.10)

project(isosm3sum)

set(PROJECT_NAME "isosm3sum")
set(PROJECT_VERSION "1.0.0")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -std=gnu11 -Wall -D_GNU_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -fPIC"
)

configure_file(include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

set(CMAKE_SYSTEM_PREFIX_PATH "/usr")
set(INCLUDEDIR "include")

if(NOT DEFINED LIBDIR)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "sw_64")
        set(LIBDIR "lib")
    else()
        set(LIBDIR "lib64")
    endif()
endif()

option(PYTHON_EXECUTABLE "Path to the Python executable" "")
if(NOT PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE "${CMAKE_SYSTEM_PREFIX_PATH}/bin/python3")
endif()

find_program(PYTHON_EXECUTABLE ${PYTHON} ${CMAKE_SYSTEM_PREFIX_PATH}/sbin ${CMAKE_SYSTEM_PREFIX_PATH}/bin)
message(STATUS "Python executable: ${PYTHON_EXECUTABLE}")

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE PYTHONSITEPACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "local/" "" PYTHONSITEPACKAGES "${PYTHONSITEPACKAGES}")
message(STATUS "Python sitepackage: ${PYTHONSITEPACKAGES}")

include_directories(${PROJECT_SOURCE_DIR}/include)

set(COMMON_SRCS src/sm3.c src/utilities.c)
set(CHECKISOSM3_SRCS src/checkisosm3.c src/libcheckisosm3.c)
set(IMPLANTISOSM3_SRCS src/implantisosm3.c src/libimplantisosm3.c)

add_executable(checkisosm3 ${COMMON_SRCS} ${CHECKISOSM3_SRCS})
add_executable(implantisosm3 ${COMMON_SRCS} ${IMPLANTISOSM3_SRCS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPT REQUIRED popt)
target_link_libraries(checkisosm3 ${POPT_LIBRARIES})
target_link_libraries(implantisosm3 ${POPT_LIBRARIES})
add_library(checkisosm3-static STATIC ${COMMON_SRCS} ${CHECKISOSM3_SRCS})
add_library(implantisosm3-static STATIC ${COMMON_SRCS} ${IMPLANTISOSM3_SRCS})

pkg_check_modules(PYTHON3 REQUIRED python3-embed)
add_library(pyisosm3sum SHARED src/pyisosm3sum.c)
set_target_properties(pyisosm3sum PROPERTIES PREFIX "")
target_link_libraries(pyisosm3sum checkisosm3-static implantisosm3-static ${PYTHON3_LIBRARIES})
target_include_directories(pyisosm3sum PUBLIC ${PYTHON3_INCLUDE_DIRS})

install(
    TARGETS checkisosm3 implantisosm3
    DESTINATION ${BINDIR}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE)

install(
    TARGETS pyisosm3sum
    DESTINATION ${PYTHONSITEPACKAGES}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE)

install(
    TARGETS checkisosm3-static implantisosm3-static
    DESTINATION ${LIBDIR}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
    ARCHIVE DESTINATION ${LIBDIR})

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${INCLUDEDIR}
    FILES_MATCHING
    PATTERN "lib*.h")

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
    FILES_MATCHING PATTERN "*.1"
)

configure_file(src/isosm3sum.pc.in ${CMAKE_BINARY_DIR}/isosm3sum.pc @ONLY)
install(
    FILES ${CMAKE_BINARY_DIR}/isosm3sum.pc
    DESTINATION ${LIBDIR}/pkgconfig
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE)

add_test(NAME testpyisosm3sum-large COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/test/testpyisosm3sum.py 200)
add_test(NAME testpyisosm3sum-small COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/test/testpyisosm3sum.py)
set_tests_properties(
    testpyisosm3sum-large testpyisosm3sum-small
    PROPERTIES ENVIRONMENT
               "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:${LD_LIBRARY_PATH};PYTHONPATH=${CMAKE_BINARY_DIR}:${PYTHONPATH}")
enable_testing()

add_custom_target(
    archive
    COMMAND git archive --format=tar --prefix=${PROJECT_NAME}-${PROJECT_VERSION}/ HEAD | bzip2 >
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.bz2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Creating archive"
    VERBATIM)

configure_file(misc/isosm3sum.spec.in ${CMAKE_BINARY_DIR}/isosm3sum.spec)
add_custom_target(
    srpm
    COMMAND make archive && bash ${PROJECT_SOURCE_DIR}/scripts/generate-srpm.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating source rpm"
    VERBATIM)
