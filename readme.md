# cmark-gfm-tcl

A Tcl/C/C++ extension for rendering Markdown to HTML with GFM tables and normalizing CommonMark. It uses the external cmark-gfm core and extension libraries.

This extension was generated with help from AI.

The extension supports Tcl 8.6 and Tcl 9, with a separate build for each major ABI. Shared builds use Tcl stubs and provide a loadable `cmark_gfm` package. Static builds support embedding in Tcl hosts and native C/C++ applications.

## Tcl API

```tcl
package require cmark_gfm 0.1.0
puts [::cmark_gfm::render_gfm_html {# Hello}]
# <h1>Hello</h1>
puts [::cmark_gfm::normalize_commonmark {  **hello**  }]
# **hello**
```

| Command | Result |
| --- | --- |
| `::cmark_gfm::render_gfm_html markdown` | HTML, including GFM pipe tables; preserves cmark's trailing newline. |
| `::cmark_gfm::normalize_commonmark markdown` | Canonical CommonMark with leading/trailing ASCII whitespace removed. |

Both commands take exactly one argument. Empty input returns an empty string. Normalization trims before parsing, uses render width zero, and returns the trimmed input if cmark produces empty output (for example, reference definitions alone). It does not attach GFM extensions. HTML rendering attaches only `table`; strikethrough, autolinks, and task lists are not enabled.

Failures raise Tcl errors with `CMARK_GFM RENDER`, `CMARK_GFM NORMALIZE`, or `CMARK_GFM SIZE`; wrong argument counts use `TCL WRONGARGS`.

## Build and test

Building requires C11 and C++17 compilers, CMake 3.22.1 or newer, Tcl development headers/stubs, and cmark-gfm's core and extension headers/libraries. `/usr/local` is searched by CMake by default:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
TCLLIBPATH="$PWD/build" tclsh9.1 examples/render.tcl
cmake --install build --prefix /your/install/prefix
```

Use the `tclsh` matching the selected headers (shown in the CMake cache). To choose Tcl 8.6 explicitly, for example:

```sh
cmake -S . -B build-86 \
  -DTCL_INCLUDE_PATH=/usr/include/tcl8.6 \
  -DTCL_STUB_LIBRARY=/usr/lib/x86_64-linux-gnu/libtclstub8.6.a \
  -DTCL_TCLSH=/usr/bin/tclsh8.6
cmake --build build-86 --parallel
ctest --test-dir build-86 --output-on-failure
```

`TCL_INCLUDE_DIR` and `TCL_LIBRARY_DIR` are also supported. Use `-DCmarkGfm_ROOT=/your/cmark/prefix` or `CMAKE_PREFIX_PATH` for cmark-gfm elsewhere. Both upstream libraries are statically linked by default, so archives used in a shared extension must be built with `CMAKE_POSITION_INDEPENDENT_CODE=ON`. Set `CMARK_GFM_USE_STATIC_LIBS=OFF` in a fresh build directory to use shared upstream libraries; make those libraries available to the runtime loader. No cmark-gfm source is bundled or downloaded during configuration.

Public headers are installed under `include/cmark-gfm-tcl`; the library and optional package index under `lib/tclcmarkgfm0.1.0` (subject to `CMAKE_INSTALL_LIBDIR`). Add the installed package directory to Tcl's `auto_path` or `TCLLIBPATH` when using a custom prefix.

## Static embedding

```sh
cmake -S . -B build-static -DBUILD_SHARED_LIBS=OFF
cmake --build build-static --parallel
ctest --test-dir build-static --output-on-failure
cmake --install build-static --prefix /your/static/prefix
```

The static build defines `STATIC_BUILD`, does not use Tcl stubs, and installs no `pkgIndex.tcl`. A Tcl host links the real Tcl library and registers the extension after `Tcl_Init`:

```c
#include <library.h>

/* Tcl 9; on Tcl 8.6 use Tcl_StaticPackage instead. */
Tcl_StaticLibrary(NULL, "Tclcmarkgfm", Tclcmarkgfm_Init, NULL);
/* The Tcl application then runs: load {} Tclcmarkgfm */
/* Commands are now available through: package require cmark_gfm */
```

See `tests/static_shell.c` for a complete host, including Tcl 9's ZIP filesystem initialization. Set `TCL_LIBRARY` explicitly if the test host's real Tcl library needs selecting. CTest runs the same Tcl suite in this embedded host, as well as native C and C++ tests.

## C and C++ APIs

The installed CMake package exports `cmark_gfm::tclcmarkgfm` with its native dependencies:

```cmake
find_package(cmark-gfm-tcl CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE cmark_gfm::tclcmarkgfm)
```

For static native C/C++ use, no Tcl symbols are pulled from the archive unless the host calls `Tclcmarkgfm_Init`. A Tcl host additionally supplies Tcl include directories and links Tcl. A C executable must use a C++ linker because the renderer is implemented in C++.

```c
#include <cmark_gfm.h>
#include <stdio.h>

const char *error = NULL;
char *html = cmark_gfm_render_html("# Hello", 7, &error);
if (html != NULL) {
    puts(html);
    cmark_gfm_free(html);
} else {
    fprintf(stderr, "%s\n", error);
}
```

`cmark_gfm_normalize_commonmark` has the same signature. Input is length-delimited UTF-8; `NULL` input is valid only with length zero. Success returns an allocated NUL-terminated string, including for empty output. Always release it using `cmark_gfm_free` (which also accepts `NULL`). The optional error pointer receives a borrowed static message on failure and `NULL` on success. No C++ exceptions escape these C functions.

```cpp
#include <markdown_renderer.h>

std::string html;
cmark_gfm::MarkdownError error;
if (!cmark_gfm::render_gfm_html("# Hello", html, error)) {
    // error.message is a borrowed static string.
}
```

`cmark_gfm::normalize_commonmark` has the same output/error convention. C++ functions return `false` for renderer failures, leave output unchanged on failure, and clear the error message on success. Standard allocation/system exceptions may propagate to C++ callers. The library guards its own first extension registration; hosts using upstream cmark-gfm directly must coordinate their registration and must not release the global plugin registry while this library is in use. Avoid loading CommonMark `cmark` and `cmark-gfm` into the same native symbol namespace: upstream libraries share C symbol names.

## Rendering and input handling

cmark-gfm's default HTML rendering omits raw HTML and unsafe link destinations; `CMARK_OPT_UNSAFE` is never enabled. The extension does not read files or execute Tcl scripts supplied in Markdown. Tcl strings are converted to/from standard UTF-8 so embedded NUL and supplementary characters are handled correctly. cmark replaces NUL with U+FFFD. Normalization also replaces invalid UTF-8 with U+FFFD.

There is no input, CPU, or total output quota. Hosts must enforce suitable limits and choose sanitization for their output context. Normalized Markdown is not sanitized HTML. No `Tclcmarkgfm_SafeInit` entry point is provided.
