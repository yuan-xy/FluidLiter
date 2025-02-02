cmake_minimum_required(VERSION 3.1)

project(fluidlite
    LANGUAGES C
    VERSION 1.2.2
)

include(GNUInstallDirs)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(WITH_FLOAT "Use 32 bit float type samples (instead of 64 bit double type)" TRUE)
option(CMAKE_POSITION_INDEPENDENT_CODE "Use PIC for building all sources" TRUE)

string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWERCASE_BUILD_TYPE)
if("${LOWERCASE_BUILD_TYPE}" STREQUAL "debug")
    set(DEBUG TRUE)
endif()
include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)
include(CheckIncludeFiles)
check_include_files("stdlib.h;stdio.h;stdarg.h;string.h;float.h;limits.h;math.h" STDC_HEADERS)
set(HAVE_STDLIB_H ${STDC_HEADERS} CACHE INTERNAL "Have include stdlib.h")
set(HAVE_STDIO_H ${STDC_HEADERS} CACHE INTERNAL "Have include stdio.h")
set(HAVE_STDARG_H ${STDC_HEADERS} CACHE INTERNAL "Have include stdarg.h")
set(HAVE_STRING_H ${STDC_HEADERS} CACHE INTERNAL "Have include string.h")
set(HAVE_FLOAT_H ${STDC_HEADERS} CACHE INTERNAL "Have include float.h")
set(HAVE_LIMITS_H ${STDC_HEADERS} CACHE INTERNAL "Have include limits.h")
set(HAVE_MATH_H ${STDC_HEADERS} CACHE INTERNAL "Have include math.h")
include(CheckIncludeFile)
check_include_file(fcntl.h HAVE_FCNTL_H)

list(APPEND HEADERS
    include/fluidlite.h
)

list(APPEND SCOPED_HEADERS
    include/fluidlite/types.h
    include/fluidlite/synth.h
    include/fluidlite/sfont.h
    include/fluidlite/log.h
    include/fluidlite/misc.h
    include/fluidlite/mod.h
    include/fluidlite/gen.h
    include/fluidlite/voice.h
)

list(APPEND SOURCES
    src/fluid_chan.c
    src/fluid_conv.c
    src/fluid_defsfont.c
    src/fluid_dsp_float.c
    src/fluid_gen.c
    src/fluid_hash.c
    src/fluid_list.c
    src/fluid_mod.c
    src/fluid_rev.c
    src/fluid_chorus.c
    src/fluid_synth.c
    src/fluid_log.c
    src/fluid_tuning.c
    src/fluid_voice.c
)


configure_file(src/fluid_config.cmake ${PROJECT_BINARY_DIR}/fluid_config.h @ONLY)

# The interface library collects all PRIVATE build options for the static and dynamic targets

add_library(${PROJECT_NAME}-options INTERFACE)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME}-options INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(${PROJECT_NAME}-options INTERFACE ${PROJECT_BINARY_DIR})
target_include_directories(${PROJECT_NAME}-options INTERFACE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME}-options INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Dependencies:

set(ADDITIONAL_LIBS)
set(PC_LIBS)
set(PC_REQUIRES)

# find the math lib, except in macOS/Windows
if (UNIX AND NOT APPLE)
    find_library(M_LIBRARY m)
    message(STATUS "Math library: ${M_LIBRARY}")
    if(M_LIBRARY)
        list(APPEND ADDITIONAL_LIBS ${M_LIBRARY})
        list(APPEND PC_LIBS -lm)
    endif()
endif()

set(FLUIDLITE_VENDORED FALSE)

# When defined, respect CMake's BUILD_SHARED_LIBS setting
set(FLUIDLITE_STATIC_ENABLED_BY_DEFAULT ON)
set(FLUIDLITE_SHARED_ENABLED_BY_DEFAULT OFF)

if(BUILD_SHARED_LIBS)
    set(FLUIDLITE_SHARED_ENABLED_BY_DEFAULT ON)
    set(FLUIDLITE_STATIC_ENABLED_BY_DEFAULT OFF)
endif()

# Static library target

option(FLUIDLITE_BUILD_STATIC "Build static library" ${FLUIDLITE_STATIC_ENABLED_BY_DEFAULT})
if(FLUIDLITE_BUILD_STATIC)
    add_library(${PROJECT_NAME}-static STATIC ${SOURCES})
    set_target_properties(${PROJECT_NAME}-static PROPERTIES C_STANDARD 99)
    target_compile_definitions(${PROJECT_NAME}-static PUBLIC FLUIDLITE_STATIC)
    target_link_libraries(${PROJECT_NAME}-static PRIVATE $<BUILD_INTERFACE:${PROJECT_NAME}-options>)
    target_link_libraries(${PROJECT_NAME}-static PUBLIC ${ADDITIONAL_LIBS})
    if(MSVC OR (WATCOM AND (WIN32 OR OS2)))
        set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-static)
    else()
        set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    endif()
    target_include_directories(${PROJECT_NAME}-static PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include;${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
endif()

# Shared dynamic library target

option(FLUIDLITE_BUILD_SHARED "Build shared library" ${FLUIDLITE_SHARED_ENABLED_BY_DEFAULT})
if(FLUIDLITE_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 99)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FLUIDLITE_DLL_EXPORTS)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${PROJECT_NAME}-options>)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ADDITIONAL_LIBS})
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include;${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
endif()

if((NOT FLUIDLITE_BUILD_SHARED) AND (NOT FLUIDLITE_BUILD_STATIC))
    message(FATAL_ERROR "Neither dynamic nor static library build is selected.")
endif()

string(JOIN " " PC_LIBS ${PC_LIBS})
string(JOIN " " PC_REQUIRES ${PC_REQUIRES})

if(IS_ABSOLUTE ${CMAKE_INSTALL_LIBDIR})
    set(fluidlite_libdir "${CMAKE_INSTALL_LIBDIR}")
else()
    set(fluidlite_libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE ${CMAKE_INSTALL_INCLUDEDIR})
    set(fluidlite_includedir "${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(fluidlite_includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
configure_file(fluidlite.pc.in ${PROJECT_BINARY_DIR}/fluidlite.pc @ONLY)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${SCOPED_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fluidlite)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fluidlite.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)


# Exported targets

if(FLUIDLITE_BUILD_STATIC)
    install(TARGETS ${PROJECT_NAME}-static
        EXPORT ${PROJECT_NAME}-static-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ${PROJECT_NAME}-static-targets
        FILE ${PROJECT_NAME}-static-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif()

if(FLUIDLITE_BUILD_SHARED)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-shared-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ${PROJECT_NAME}-shared-targets
        FILE ${PROJECT_NAME}-shared-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${PROJECT_NAME}-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
	${PROJECT_NAME}-config.cmake.in
	${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
