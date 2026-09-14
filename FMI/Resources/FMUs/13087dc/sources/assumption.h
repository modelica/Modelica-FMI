#if (defined(DYMOLA_STANDALONE) && defined(DEBUG)) || (defined(EXTENDED_KERNEL) && !defined(CNEXT_CLIENT))
/* Only activated if you manually add the define 'DEBUG' */
#include <assert.h>

#define assumption(x) do{if (!(x)) {assert(x);/*ReportInternalError();throw AssertionFailed;*/}}while(0)

/* For errors that only need to be checked during development, will fall thru to other code in release mode and be handled gracefully */
#define assumptionDevelopment(x) assert(x)
#else

#define assumption(x) do{;}while(0)
#define assumptionDevelopment(x) do{;}while(0)
#endif
