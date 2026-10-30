/* Copyright (c) 2026 Neofytos Dimitriou. SPDX-License-Identifier: MIT */
#ifndef TCLCMARKGFM_MARKDOWN_RENDERER_H
#define TCLCMARKGFM_MARKDOWN_RENDERER_H

#include <string>
#include <string_view>
#include "tclcmarkgfm_export.h"

namespace cmark_gfm {

struct MarkdownError {
    // Borrowed static message; nullptr after a successful call.
    const char *message = nullptr;
};

// On failure, output is unchanged and error describes the failure.
// Standard C++ allocation/system exceptions may propagate to C++ callers.
TCLCMARKGFM_EXPORT bool render_gfm_html(std::string_view markdown,
                                     std::string &html_out, MarkdownError &error);
TCLCMARKGFM_EXPORT bool normalize_commonmark(std::string_view markdown,
                                          std::string &markdown_out, MarkdownError &error);

}  // namespace cmark_gfm

#endif
