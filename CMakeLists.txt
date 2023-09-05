cmake_minimum_required(VERSION 3.10)

project(isosm3sum)

set(PROJECT_NAME "isosm3sum")
set(PROJECT_VERSION "0.7.0")

configure_file(include/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h)

set(CMAKE_SYSTEM_PREFIX_PATH "/usr")
set(INCLUDEDIR "include")

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
