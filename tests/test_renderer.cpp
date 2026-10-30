#include "markdown_renderer.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {
int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        std::cerr << message << '\n';
        ++failures;
    }
}
}  // namespace

int main() {
    // Race the very first table render, before any other test registers GFM.
    std::atomic<bool> start{false};
    std::atomic<bool> concurrent_ok{true};
    std::vector<std::thread> workers;
    for (int i = 0; i < 8; ++i) {
        workers.emplace_back([&] {
            while (!start.load()) {
                std::this_thread::yield();
            }
            for (int iteration = 0; iteration < 50; ++iteration) {
                std::string html;
                cmark_gfm::MarkdownError error;
                if (!cmark_gfm::render_gfm_html("| A | B |\n| - | - |\n| 1 | 2 |\n", html, error) ||
                    html.find("<table>") == std::string::npos ||
                    html.find("<td>1</td>") == std::string::npos) {
                    concurrent_ok = false;
                }
            }
        });
    }
    start = true;
    for (auto &worker : workers) {
        worker.join();
    }
    check(concurrent_ok, "concurrent table rendering failed");

    std::string output;
    cmark_gfm::MarkdownError error{"stale error"};
    check(cmark_gfm::render_gfm_html("# Title\n\n**bold** & < >", output, error), "render failed");
    check(output == "<h1>Title</h1>\n<p><strong>bold</strong> &amp; &lt; &gt;</p>\n",
          "heading/emphasis/escaping changed");
    check(error.message == nullptr, "successful render did not clear error");
    check(cmark_gfm::render_gfm_html({}, output, error) && output.empty(), "empty HTML failed");
    check(cmark_gfm::render_gfm_html("<script>alert(1)</script>", output, error) &&
          output.find("<script>") == std::string::npos, "raw HTML was emitted");
    check(cmark_gfm::render_gfm_html("[unsafe](javascript:alert%281%29)", output, error) &&
          output.find("javascript:") == std::string::npos, "unsafe URL was emitted");
    check(cmark_gfm::render_gfm_html("caf\xc3\xa9 \xf0\x9f\x98\x80", output, error) &&
          output == "<p>caf\xc3\xa9 \xf0\x9f\x98\x80</p>\n", "UTF-8 rendering changed");
    const std::string nul_input("a\0b", 3);
    check(cmark_gfm::render_gfm_html(nul_input, output, error) &&
          output == "<p>a\xef\xbf\xbd" "b</p>\n", "NUL truncated input");

    check(cmark_gfm::normalize_commonmark("  plain text  ", output, error) &&
          output == "plain text", "normalization did not trim ASCII whitespace");
    check(cmark_gfm::normalize_commonmark(" \t\r\n\f\v", output, error) && output.empty(),
          "whitespace-only normalization failed");
    check(cmark_gfm::normalize_commonmark("Title\n=====\n", output, error) && output == "# Title",
          "setext heading was not normalized");
    check(cmark_gfm::normalize_commonmark("[ref]: /url", output, error) && output == "[ref]: /url",
          "empty-render fallback changed");
    check(cmark_gfm::normalize_commonmark("a\xff" "b", output, error) &&
          output == "a\xef\xbf\xbd" "b", "invalid UTF-8 was not replaced");
    output = "  **same**  ";
    check(cmark_gfm::normalize_commonmark(output, output, error) && output == "**same**",
          "input/output aliasing failed");
    check(error.message == nullptr, "successful normalization did not clear error");
    return failures == 0 ? 0 : 1;
}
