/* Copyright (c) 2026 Neofytos Dimitriou. SPDX-License-Identifier: MIT */
#include "library.h"
#include "cmark_gfm.h"

#include <limits.h>
#include <string.h>

#if TCL_MAJOR_VERSION < 9
typedef int CmarkTclSize;
#define CMARK_TCL_SIZE_MAX INT_MAX
#define CMARK_TCL_MIN_VERSION "8.6"
#else
typedef Tcl_Size CmarkTclSize;
#define CMARK_TCL_SIZE_MAX TCL_SIZE_MAX
#define CMARK_TCL_MIN_VERSION "9.0"
#endif

static int convert_command(Tcl_Interp *interp, int objc, Tcl_Obj *const objv[], int normalize) {
    CmarkTclSize length;
    const char *input;
    const char *message = NULL;
    char *output;
    size_t output_length;
    Tcl_Encoding encoding;
    Tcl_DString utf8_input;
    Tcl_DString tcl_output;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "markdown");
        return TCL_ERROR;
    }
    encoding = Tcl_GetEncoding(interp, "utf-8");
    if (encoding == NULL) {
        return TCL_ERROR;
    }
    input = Tcl_GetStringFromObj(objv[1], &length);
    Tcl_DStringInit(&utf8_input);
    // Tcl's internal UTF-8 differs from standard UTF-8 (notably NUL and emoji).
    Tcl_UtfToExternalDString(encoding, input, length, &utf8_input);
    if (normalize) {
        output = cmark_gfm_normalize_commonmark(Tcl_DStringValue(&utf8_input),
                    (size_t)Tcl_DStringLength(&utf8_input), &message);
    } else {
        output = cmark_gfm_render_html(Tcl_DStringValue(&utf8_input),
                    (size_t)Tcl_DStringLength(&utf8_input), &message);
    }
    Tcl_DStringFree(&utf8_input);
    if (output == NULL) {
        Tcl_FreeEncoding(encoding);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(message, -1));
        Tcl_SetErrorCode(interp, "CMARK_GFM", normalize ? "NORMALIZE" : "RENDER", NULL);
        return TCL_ERROR;
    }
    output_length = strlen(output);
    // Allow for expansion to Tcl's internal encoding before narrowing the size.
    if (output_length > (size_t)CMARK_TCL_SIZE_MAX / 3) {
        cmark_gfm_free(output);
        Tcl_FreeEncoding(encoding);
        Tcl_SetObjResult(interp, Tcl_NewStringObj("rendered output is too large for Tcl", -1));
        Tcl_SetErrorCode(interp, "CMARK_GFM", "SIZE", NULL);
        return TCL_ERROR;
    }
    Tcl_DStringInit(&tcl_output);
    Tcl_ExternalToUtfDString(encoding, output, (CmarkTclSize)output_length, &tcl_output);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(Tcl_DStringValue(&tcl_output),
                                            Tcl_DStringLength(&tcl_output)));
    Tcl_DStringFree(&tcl_output);
    cmark_gfm_free(output);
    Tcl_FreeEncoding(encoding);
    return TCL_OK;
}

static int render_command(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    (void)data;
    return convert_command(interp, objc, objv, 0);
}

static int normalize_command(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    (void)data;
    return convert_command(interp, objc, objv, 1);
}

int Tclcmarkgfm_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, CMARK_TCL_MIN_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tcl_FindNamespace(interp, "::cmark_gfm", NULL, TCL_GLOBAL_ONLY) == NULL &&
        Tcl_CreateNamespace(interp, "::cmark_gfm", NULL, NULL) == NULL) {
        return TCL_ERROR;
    }
    if (Tcl_PkgProvide(interp, "cmark_gfm", CMARK_GFM_TCL_VERSION) != TCL_OK) {
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "::cmark_gfm::render_gfm_html", render_command, NULL, NULL);
    Tcl_CreateObjCommand(interp, "::cmark_gfm::normalize_commonmark", normalize_command, NULL, NULL);
    return TCL_OK;
}
