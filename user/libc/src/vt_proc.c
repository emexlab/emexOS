#include <emx/tty.h>
#include "_syscall.h"

#define _SCAL_VT_SETPROC 251

int vt_setproc(unsigned long long vt_id) {
    return (int)_sc1(_SCAL_VT_SETPROC, (long)vt_id);
}