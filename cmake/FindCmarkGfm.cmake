# cmark-gfm is required, including its extension library. CommonMark-only cmark
# is not a substitute. Set CmarkGfm_ROOT (or CMAKE_PREFIX_PATH) for a custom prefix.
# CMARK_GFM_USE_STATIC_LIBS selects archives, including when building a shared Tcl
# extension; those archives must have been built with position-independent code.
find_path(CMARK_GFM_INCLUDE_DIR NAMES cmark-gfm.h PATH_SUFFIXES cmark-gfm)
find_path(CMARK_GFM_EXTENSIONS_INCLUDE_DIR NAMES cmark-gfm-core-extensions.h
    PATH_SUFFIXES cmark-gfm)

if(CMARK_GFM_USE_STATIC_LIBS)
    set(_cmark_gfm_core_names "${CMAKE_STATIC_LIBRARY_PREFIX}cmark-gfm${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(_cmark_gfm_extension_names
        "${CMAKE_STATIC_LIBRARY_PREFIX}cmark-gfm-extensions${CMAKE_STATIC_LIBRARY_SUFFIX}"
        "${CMAKE_STATIC_LIBRARY_PREFIX}cmark-gfmextensions${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(_cmark_gfm_core_names cmark-gfm)
    set(_cmark_gfm_extension_names cmark-gfm-extensions cmark-gfmextensions)
endif()
find_library(CMARK_GFM_LIBRARY NAMES ${_cmark_gfm_core_names})
find_library(CMARK_GFM_EXTENSIONS_LIBRARY NAMES ${_cmark_gfm_extension_names})
unset(_cmark_gfm_core_names)
unset(_cmark_gfm_extension_names)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CmarkGfm REQUIRED_VARS
    CMARK_GFM_INCLUDE_DIR CMARK_GFM_EXTENSIONS_INCLUDE_DIR
    CMARK_GFM_LIBRARY CMARK_GFM_EXTENSIONS_LIBRARY)

if(CmarkGfm_FOUND AND NOT TARGET CmarkGfm::Core)
    add_library(CmarkGfm::Core UNKNOWN IMPORTED)
    set_target_properties(CmarkGfm::Core PROPERTIES
        IMPORTED_LOCATION "${CMARK_GFM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMARK_GFM_INCLUDE_DIR}")
    add_library(CmarkGfm::Extensions UNKNOWN IMPORTED)
    set_target_properties(CmarkGfm::Extensions PROPERTIES
        IMPORTED_LOCATION "${CMARK_GFM_EXTENSIONS_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMARK_GFM_EXTENSIONS_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES CmarkGfm::Core)
    if(CMARK_GFM_USE_STATIC_LIBS)
        set_property(TARGET CmarkGfm::Core PROPERTY
            INTERFACE_COMPILE_DEFINITIONS CMARK_GFM_STATIC_DEFINE)
        set_property(TARGET CmarkGfm::Extensions PROPERTY
            INTERFACE_COMPILE_DEFINITIONS CMARK_GFM_EXTENSIONS_STATIC_DEFINE)
    endif()
endif()
mark_as_advanced(CMARK_GFM_INCLUDE_DIR CMARK_GFM_EXTENSIONS_INCLUDE_DIR
    CMARK_GFM_LIBRARY CMARK_GFM_EXTENSIONS_LIBRARY)
