#if defined CONFIG_MMU
#include "head_std.S"
#else
#include "head-atarinommu.S"
#endif
