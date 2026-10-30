#include "library.h"

static int app_init(Tcl_Interp *interp) {
    if (Tcl_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }
#if TCL_MAJOR_VERSION >= 9
    Tcl_StaticLibrary(NULL, "Tclcmarkgfm", Tclcmarkgfm_Init, NULL);
#else
    Tcl_StaticPackage(NULL, "Tclcmarkgfm", Tclcmarkgfm_Init, NULL);
#endif
    return Tcl_Eval(interp, "load {} Tclcmarkgfm");
}

int main(int argc, char **argv) {
#if TCL_MAJOR_VERSION >= 9
    TclZipfs_AppHook(&argc, &argv);
#endif
    Tcl_Main(argc, argv, app_init);
    return 0;
}
