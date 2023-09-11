cmake_minimum_required(VERSION 3.10)

project(isosm3sum)

set(PROJECT_NAME "isosm3sum")
set(PROJECT_VERSION "0.8.0")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -std=gnu11 -Wall -D_GNU_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -fPIC"
)

configure_file(include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

set(CMAKE_SYSTEM_PREFIX_PATH "/usr")
set(INCLUDEDIR "include")

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
