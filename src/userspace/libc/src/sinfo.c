#include "_syscall.h"
#include <emx/sinfo.h>

int sysinfo(struct sysinfo_t *info) { (int)_sc1(_SCAL_SYSINFO, (long)info); }