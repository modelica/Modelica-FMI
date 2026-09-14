#ifndef LIBFMI_LIB
/* This is a wrapper file including all source files. Use this when you need to compile everything into a single
   compilation unit, where all internal functions and variables are static. This is necessary when combining
   the source code with others FMUs exported by CATIA DBM or Dymola.*/

/* set to be able to compile everything in a single compilation unit, which enables static functions
and symbols, which in turn enables creation of static libraries without name clashes */
#ifndef FMU_SOURCE_SINGLE_UNIT
#define FMU_SOURCE_SINGLE_UNIT
#endif
/* must set explicitly for FMI code */
#if !defined(DYMOLA_STATIC)
#define DYMOLA_STATIC static
#endif
#if !defined(DYMOLA_STATIC_IMPORT)
#define DYMOLA_STATIC_IMPORT extern
#endif
/* for Sundials code */
#if !defined(SUNDIALS_EXPORT)
#define SUNDIALS_EXPORT DYMOLA_STATIC
#endif
#if !defined(RT)
#define RT
#endif
#ifndef TPL_API
#define TPL_API DYMOLA_STATIC
#endif // !TPL_API


#if defined(RTTWINCAT)
#define NO_LOCALE
#define NO_EXTERNAL_DLL
#endif

#if defined(_MSC_VER) && !defined(NO_EXTERNAL_DLL)
#pragma comment(lib, "Shlwapi.lib")
#endif

#include "snprintf.c"

/*dsblock1.c has dependecies on definitions from this header*/
#include "fmiModelIdentifier.h"

/* dsmodel */
#include "dsmodel.c"
#include "extendedIncludes.h"

/* libfmi */
#if (FMI_VERSION >= FMI_3)
#include "fmi3Functions.c"
#elif (FMI_VERSION >= FMI_2)
#include "fmi2Functions.c"
#else
#include "fmiFunctions.c"
#endif
#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
#include "tpl.c"
//#include "mmap.c"
#endif

#include "fmiCommonFunctions_int.c"
#include "fmiMEFunctions_int.c"
#include "util.c"
#include "fmiCoSimFunctions_int.c"

#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
#include "integration.c"
#include "jac.c"

/* CVODE */
#include "cvode.c"
#include "cvode_direct.c"
#include "cvode_io.c"
#include "cvode_ls.c"
#include "cvode_nls.c"
#include "cvode_proj.c"
#include "ds_sundials_extensions.c"
#include "nvector_serial.c"
#include "sundials_context.c"
#include "sundials_dense.c"
#include "sundials_direct.c"
#include "sundials_linearsolver.c"
#include "sundials_logger.c"
#include "sundials_math.c"
#include "sundials_matrix.c"
#include "sundials_nonlinearsolver.c"
#include "sundials_nvector.c"
#include "sundials_nvector_senswrapper.c"
#include "sundials_version.c"
#include "sunlinsol_dense.c"
#include "sunmatrix_band.c"
#include "sunmatrix_dense.c"
#include "sunnonlinsol_newton.c"
#ifdef FMU_SOURCE_CODE_EXPORT_SPARSE
#include "ds_sundials_extensions_sparse.c"
#include "sunlinsol_superlumt.c"
#include "sunmatrix_sparse.c"
#endif /* FMU_SOURCE_CODE_EXPORT_SPARSE */
#ifdef INCLUDE_SUNDIALS_IDA
/* IDA */
#include "ida.c"
#include "ida_direct.c"
#include "ida_ic.c"
#include "ida_io.c"
#include "ida_ls.c"
#include "ida_nls.c"
#endif /* INCLUDE_SUNDIALS_IDA */

#endif /*ONLY_INCLUDE_INLINE_INTEGRATION*/
#endif
