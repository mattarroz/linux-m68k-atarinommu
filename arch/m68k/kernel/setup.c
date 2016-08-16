#ifdef CONFIG_ATARI
#include "setup_mm.c"
#else
#ifdef CONFIG_MMU
#include "setup_mm.c"
#else
#include "setup_no.c"
#endif
#endif
