#include "cmark_gfm.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "%s\n", message);
        ++failures;
    }
}

int main(void) {
    const char *error = "stale error";
    const char input[] = {'#', ' ', 'H', 'x'};
    char *text = cmark_gfm_render_html(input, 3, &error);
    check(text != NULL && strcmp(text, "<h1>H</h1>\n") == 0, "length-delimited rendering failed");
    check(error == NULL, "successful C call did not clear error");
    cmark_gfm_free(text);

    text = cmark_gfm_normalize_commonmark("  text  ", 8, &error);
    check(text != NULL && strcmp(text, "text") == 0 && error == NULL, "C normalization failed");
    cmark_gfm_free(text);

    text = cmark_gfm_render_html(NULL, 0, &error);
    check(text != NULL && text[0] == '\0' && error == NULL, "empty C rendering failed");
    cmark_gfm_free(text);
    text = cmark_gfm_normalize_commonmark(NULL, 0, NULL);
    check(text != NULL && text[0] == '\0', "empty C normalization failed");
    cmark_gfm_free(text);

    text = cmark_gfm_render_html(NULL, 1, &error);
    check(text == NULL && error != NULL, "invalid C render input was accepted");
    text = cmark_gfm_normalize_commonmark(NULL, 1, &error);
    check(text == NULL && error != NULL, "invalid C normalization input was accepted");
    check(cmark_gfm_render_html(NULL, 1, NULL) == NULL, "optional error pointer failed");

    text = cmark_gfm_render_html("a\0b", 3, &error);
    check(text != NULL && strcmp(text, "<p>a\xef\xbf\xbd" "b</p>\n") == 0,
          "embedded NUL truncated C input");
    cmark_gfm_free(text);
    cmark_gfm_free(NULL);
    return failures == 0 ? 0 : 1;
}
