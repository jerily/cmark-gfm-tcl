/* Copyright (c) 2026 Neofytos Dimitriou. SPDX-License-Identifier: MIT */
#include "cmark_gfm.h"
#include "markdown_renderer.h"

#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace {

using RenderFunction = bool (*)(std::string_view, std::string &, cmark_gfm::MarkdownError &);

char *convert(const char *markdown, size_t length, const char **error, RenderFunction render) noexcept {
    if (error != nullptr) {
        *error = nullptr;
    }
    if (markdown == nullptr && length != 0) {
        if (error != nullptr) {
            *error = "markdown must not be NULL when length is nonzero";
        }
        return nullptr;
    }
    const char *message = nullptr;
    try {
        std::string output;
        cmark_gfm::MarkdownError render_error;
        if (render(std::string_view(markdown == nullptr ? "" : markdown, length), output, render_error)) {
            auto *result = static_cast<char *>(std::malloc(output.size() + 1));
            if (result != nullptr) {
                std::memcpy(result, output.c_str(), output.size() + 1);
                return result;
            }
            message = "out of memory while processing markdown";
        } else {
            message = render_error.message;
        }
    } catch (const std::bad_alloc &) {
        message = "out of memory while processing markdown";
    } catch (const std::length_error &) {
        message = "markdown input or output is too large";
    } catch (...) {
        message = "unexpected failure while processing markdown";
    }
    if (error != nullptr) {
        *error = message;
    }
    return nullptr;
}

}  // namespace

extern "C" char *cmark_gfm_render_html(const char *markdown, size_t length, const char **error) {
    return convert(markdown, length, error, cmark_gfm::render_gfm_html);
}

extern "C" char *cmark_gfm_normalize_commonmark(const char *markdown, size_t length, const char **error) {
    return convert(markdown, length, error, cmark_gfm::normalize_commonmark);
}

extern "C" void cmark_gfm_free(char *text) {
    std::free(text);
}
