/* Extracted from jerily-webserver/apps/markdown-commons.
 * Copyright (c) 2026 Neofytos Dimitriou. SPDX-License-Identifier: MIT */
#include "markdown_renderer.h"

#include <memory>
#include <mutex>

extern "C" {
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>
}

namespace cmark_gfm {
namespace {

using Parser = std::unique_ptr<cmark_parser, decltype(&cmark_parser_free)>;
using Document = std::unique_ptr<cmark_node, decltype(&cmark_node_free)>;

struct RenderedDeleter {
    void operator()(char *text) const {
        cmark_get_default_mem_allocator()->free(text);
    }
};
using Rendered = std::unique_ptr<char, RenderedDeleter>;

std::string_view trim_ascii(std::string_view text) {
    constexpr std::string_view whitespace = " \t\n\r\f\v";
    const auto begin = text.find_first_not_of(whitespace);
    if (begin == std::string_view::npos) {
        return {};
    }
    return text.substr(begin, text.find_last_not_of(whitespace) - begin + 1);
}

}  // namespace

bool render_gfm_html(std::string_view markdown, std::string &html_out, MarkdownError &error) {
    constexpr int options = CMARK_OPT_DEFAULT;
    // Upstream's registration guard is a plain static int, not a thread guard.
    static std::once_flag registered;
    std::call_once(registered, cmark_gfm_core_extensions_ensure_registered);

    Parser parser(cmark_parser_new(options), cmark_parser_free);
    if (!parser) {
        error.message = "failed to initialize markdown parser";
        return false;
    }
    auto *table = cmark_find_syntax_extension("table");
    if (table == nullptr || !cmark_parser_attach_syntax_extension(parser.get(), table)) {
        error.message = "failed to enable markdown table extension";
        return false;
    }

    cmark_parser_feed(parser.get(), markdown.empty() ? "" : markdown.data(), markdown.size());
    Document document(cmark_parser_finish(parser.get()), cmark_node_free);
    if (!document) {
        error.message = "failed to parse markdown with GitHub Flavored Markdown";
        return false;
    }
    Rendered rendered(cmark_render_html(document.get(), options,
                                       cmark_parser_get_syntax_extensions(parser.get())));
    if (!rendered) {
        error.message = "failed to render markdown with GitHub Flavored Markdown";
        return false;
    }
    html_out = rendered.get();
    error.message = nullptr;
    return true;
}

bool normalize_commonmark(std::string_view markdown, std::string &markdown_out, MarkdownError &error) {
    const auto trimmed = trim_ascii(markdown);
    if (trimmed.empty()) {
        markdown_out.clear();
        error.message = nullptr;
        return true;
    }
    Document document(cmark_parse_document(trimmed.data(), trimmed.size(),
                                          CMARK_OPT_DEFAULT | CMARK_OPT_VALIDATE_UTF8),
                      cmark_node_free);
    if (!document) {
        error.message = "failed to parse markdown as CommonMark";
        return false;
    }
    Rendered rendered(cmark_render_commonmark(document.get(), CMARK_OPT_DEFAULT, 0));
    if (!rendered) {
        error.message = "failed to normalize markdown as CommonMark";
        return false;
    }
    const auto normalized = trim_ascii(rendered.get());
    markdown_out = normalized.empty() ? trimmed : normalized;
    error.message = nullptr;
    return true;
}

}  // namespace cmark_gfm
