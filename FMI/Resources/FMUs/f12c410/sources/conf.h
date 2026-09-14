/* #define-lines below added during FMU creation */ 
#ifndef FMI_VER 
#define FMI_VER 2 
#endif 
#define FMU_CO_SIM 
#define FMU_SOURCE_CODE_EXPORT 
/* Configuration file for Dymosim FMI implementation. */

#ifndef conf_h
#define conf_h

/* -- set to 2 or 3 for FMI version 2 and 3 respectively -- */
/* use #ifndef to allow setting from makefile */
#ifndef FMI_VER
#define FMI_VER 2
#endif

/* do not edit from here ... */
#if !defined(FMI_VERSION)
#if FMI_VER == 3
#define FMI_VERSION 300
#elif FMI_VER == 2
#define FMI_VERSION 200
#else
#define FMI_VERSION 100
#endif
#endif

#if FMI_VER >= 2
/* -- save state configuration -- */
/*#define SAVE_STATE_SUNDIALS*/
#endif

#if __cplusplus || __STDC_VERSION__ >= 199901L
#define DYM_INLINE inline
#elif _MSC_VER >= 1700
#define DYM_INLINE __inline
#else
#define DYM_INLINE
#endif


/* -- Define to only inlcude nessesary part for inline integration -- */
/*#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
#define ONLY_INCLUDE_INLINE_INTEGRATION
#endif*/

/*-- DymolaEventOptional_ should Always be 0 when using CVODE and always 1 when using--*/
/*-- inline integration with Co-Simulation.  In Model Exchange it should always be   --*/
/*-- set to 0 unless the solver will call the event handling even when there are     --*/
/*-- no events, in that case define OVERIDE_DYMOLA_EVENT_OPTIONAL                    --*/
/*#define OVERIDE_DYMOLA_EVENT_OPTIONAL*/



#ifndef DYMOLA_STATIC
#define DYMOLA_STATIC
#endif

/* Skip some parts for model exchange/co-simulation 1.0 code export since standard header files are separate. */
#if FMI_VER == 1 && defined(FMU_SOURCE_CODE_EXPORT)
#if defined(FMU_CO_SIM)
#define FMU_SKIP_MODEL_EXCHANGE
#else
#define FMU_SKIP_CO_SIM
#endif
#endif

/* ... do not edit until here */

/* -- switch to new code generation -- */
/*#define NEW_CODE_GENERATION*/

/* -- for debugging memory leaks -- */
/*#define MEMLEAK_DEBUG*/
#ifdef MEMLEAK_DEBUG
#define NCHUNKS 8092
#endif

/* -- to give extra informational messages -- */
/*#define FMU_VERBOSITY*/

/* -- define if compiled on platform without file system -- */
/*#define NO_FILE*/

/* -- define if you want to compile with _MSC_VER compiler but want to avoid shlwapi.h,
   fmu will not be able to load aditinal dll's in that case -- */
/*#define NO_EXTERNAL_DLL*/


/* -- for logging -- */
/*#define LOG_EVENTS*/
#define LOG_STATISTICS

#endif /* conf_h */
