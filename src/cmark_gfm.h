/* Copyright (c) 2026 Neofytos Dimitriou. SPDX-License-Identifier: MIT */
#ifndef TCLCMARKGFM_CMARK_GFM_H
#define TCLCMARKGFM_CMARK_GFM_H

#include <stddef.h>
#include "tclcmarkgfm_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Input is length-delimited UTF-8. NULL is valid only when length is zero.
 * Returns an owned, NUL-terminated string (including for empty output), or NULL
 * on failure. Release it with cmark_gfm_free(). If error is non-NULL, *error is
 * set to a borrowed static message on failure and NULL on success.
 * No C++ exceptions cross this interface. */
TCLCMARKGFM_EXPORT char *cmark_gfm_render_html(const char *markdown, size_t length,
                                             const char **error);
TCLCMARKGFM_EXPORT char *cmark_gfm_normalize_commonmark(const char *markdown, size_t length,
                                                      const char **error);
TCLCMARKGFM_EXPORT void cmark_gfm_free(char *text);

#ifdef __cplusplus
}
#endif

#endif
