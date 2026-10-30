cmake_minimum_required(VERSION 3.22.1)
project(cmark-gfm-tcl VERSION 0.1.0 LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build the loadable Tcl extension" ON)
option(CMARK_GFM_USE_STATIC_LIBS "Link the cmark-gfm core and extensions statically" ON)
include(CTest)
include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(TclStub 8.6 REQUIRED)
find_package(CmarkGfm REQUIRED)
find_package(Threads REQUIRED)

add_library(tclcmarkgfm src/library.c src/markdown_renderer.cpp src/c_api.cpp)
add_library(cmark_gfm::tclcmarkgfm ALIAS tclcmarkgfm)
target_compile_features(tclcmarkgfm PUBLIC cxx_std_17 PRIVATE c_std_11)
set_target_properties(tclcmarkgfm PROPERTIES
    PREFIX "lib" POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
generate_export_header(tclcmarkgfm)
target_include_directories(tclcmarkgfm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cmark-gfm-tcl>
)
target_include_directories(tclcmarkgfm PRIVATE ${TCL_INCLUDE_PATH})
target_link_libraries(tclcmarkgfm PRIVATE CmarkGfm::Extensions Threads::Threads)
target_compile_definitions(tclcmarkgfm PRIVATE TCL_THREADS
    CMARK_GFM_TCL_VERSION="${PROJECT_VERSION}")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(tclcmarkgfm PRIVATE USE_TCL_STUBS)
    target_link_libraries(tclcmarkgfm PRIVATE ${TCL_STUB_LIBRARY})
else()
    # As in tjson, the embedding host links Tcl and registers Tclcmarkgfm_Init.
    # Native C/C++ users do not pull library.c from the archive and need no Tcl.
    target_compile_definitions(tclcmarkgfm PUBLIC STATIC_BUILD TCLCMARKGFM_STATIC_DEFINE)
endif()
if(MSVC)
    target_compile_options(tclcmarkgfm PRIVATE /W4)
else()
    target_compile_options(tclcmarkgfm PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(CMARK_GFM_TCL_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/tclcmarkgfm${PROJECT_VERSION}")
install(TARGETS tclcmarkgfm EXPORT cmark-gfm-tcl-targets
    LIBRARY DESTINATION ${CMARK_GFM_TCL_INSTALL_DIR}
    ARCHIVE DESTINATION ${CMARK_GFM_TCL_INSTALL_DIR}
    RUNTIME DESTINATION ${CMARK_GFM_TCL_INSTALL_DIR})
install(FILES src/library.h src/cmark_gfm.h src/markdown_renderer.h
    "${CMAKE_CURRENT_BINARY_DIR}/tclcmarkgfm_export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmark-gfm-tcl)
if(BUILD_SHARED_LIBS)
    configure_file(pkgIndex.tcl.in pkgIndex.tcl @ONLY)
    if(CMAKE_CONFIGURATION_TYPES)
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/pkgIndex.tcl"
            INPUT "${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl")
    endif()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl"
        DESTINATION ${CMARK_GFM_TCL_INSTALL_DIR})
endif()

set(CMARK_GFM_TCL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/cmark-gfm-tcl")
configure_package_config_file(cmake/cmark-gfm-tcl-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/cmark-gfm-tcl-config.cmake"
    INSTALL_DESTINATION ${CMARK_GFM_TCL_CMAKE_DIR})
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmark-gfm-tcl-config-version.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(EXPORT cmark-gfm-tcl-targets NAMESPACE cmark_gfm::
    DESTINATION ${CMARK_GFM_TCL_CMAKE_DIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmark-gfm-tcl-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmark-gfm-tcl-config-version.cmake"
    cmake/FindCmarkGfm.cmake DESTINATION ${CMARK_GFM_TCL_CMAKE_DIR})
install(FILES readme.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

if(BUILD_TESTING)
    add_executable(renderer_tests tests/test_renderer.cpp)
    target_link_libraries(renderer_tests PRIVATE tclcmarkgfm Threads::Threads)
    add_test(NAME renderer COMMAND renderer_tests)

    add_executable(c_api_tests tests/test_c_api.c)
    target_link_libraries(c_api_tests PRIVATE tclcmarkgfm)
    set_target_properties(c_api_tests PROPERTIES LINKER_LANGUAGE CXX)
    add_test(NAME c_api COMMAND c_api_tests)

    if(BUILD_SHARED_LIBS)
        if(NOT TCL_TCLSH)
            message(FATAL_ERROR "Tests require a matching tclsh; set TCL_TCLSH or BUILD_TESTING=OFF")
        endif()
        set(CMARK_GFM_TCL_TEST_SHELL "${TCL_TCLSH}")
    else()
        string(REGEX MATCH "^[0-9]+\\.[0-9]+" TCL_ABI_VERSION "${TCL_VERSION}")
        string(REPLACE "." "" TCL_ABI_COMPACT "${TCL_ABI_VERSION}")
        find_library(TCL_LIBRARY NAMES tcl${TCL_ABI_VERSION} tcl${TCL_ABI_COMPACT} tcl
            HINTS ${TCL_LIBRARY_DIR} "${TCL_INCLUDE_PATH_PARENT}/lib" REQUIRED)
        add_executable(cmark_gfm_test_shell tests/static_shell.c)
        target_include_directories(cmark_gfm_test_shell PRIVATE ${TCL_INCLUDE_PATH})
        target_link_libraries(cmark_gfm_test_shell PRIVATE tclcmarkgfm ${TCL_LIBRARY})
        set(CMARK_GFM_TCL_TEST_SHELL "$<TARGET_FILE:cmark_gfm_test_shell>")
    endif()
    add_test(NAME tcl COMMAND "${CMARK_GFM_TCL_TEST_SHELL}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/all.tcl" "$<TARGET_FILE_DIR:tclcmarkgfm>")
endif()
