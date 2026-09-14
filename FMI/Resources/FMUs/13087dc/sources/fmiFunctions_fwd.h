#ifndef fmiFunctions_fwd_h
#define fmiFunctions_fwd_h

#if (FMI_VERSION >= FMI_3)
#include "fmi3Functions_fwd.h"
#elif (FMI_VERSION >= FMI_2)
#include "fmi2Functions_fwd.h"
#else
#include "fmiFunctions_1.0_fwd.h"
#endif

#endif /* fmiFunctions_fwd_h */
