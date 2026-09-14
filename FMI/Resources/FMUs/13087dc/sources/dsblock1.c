/* Begin dsblock1.c */

/*
 * Copyright (C) 1997-2001 Dynasim AB.
 * All rights reserved.
 *
 */

/* Start of DSblock C output.
   Version: 1.8, 1997-04-29
            1.9, 1997-07-04  relative tolerance of integrator used
                             for nonlinear equation solvers
           1.10, 1997-11-28  - stdlib.h included in order to get no conflicts
                               with a possible f2c.h in userfunc.c
                             - longjmp introduced for error messages, in order
                               that the model evaluation is stopped immediately
                             - event iteration at initial time even if
                               EventIterate_ = 0
                             - userfunc.c before RootFinder_/EventIterate_ flags
                               in order that these flags can be also set in 
                               userfunc.c (often more convenient).
           1.11, 1998-03-22  - changes for linear solvers (and tearing)
           1.12, 1999-01-25  - explicit initializer for matrices to avoid
                               initialization at runtime (increases executable size)
           1.13, 1999-03-03  - includes userdefs.h (userfunc.c was split in two)
*/
#if !defined(DSE_STRUCT)
 #if defined(DS_EMBEDDED)
  #define DSE_STRUCT sts->
 #else
  #define DSE_STRUCT
 #endif
#endif

#if !defined(NO_FILE)
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <setjmp.h>

#if (defined(__unix__) || defined(__linux__) || defined(__APPLE_CC__)) && defined(__GNUC__) && defined(FMI_MODULE_NAME)
/* need access to non-standard function dladdr */
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>
#undef __USE_GNU
#endif /* defined(UNIX) ... */
#endif /* !defined(NO_FILE) */

#include "dsblock.h"
#include "userdefs.h"
#include "userfunc.c"     /* To include user defined functions called 
							 from model equations. */

#if defined(NO_FILE)  
#if !defined(NO_FILE_SYSTEM)
#define NO_FILE_SYSTEM 1
#endif
#if !defined(NO_PID)
#define NO_PID 1
#endif
#if !defined(NO_TIME)
#define NO_TIME 1
#endif
#endif

#define MODELICA_EXTERNAL static
#include "ModelicaInternal.c"   /* Utility functions in Modelica 2.1 */
#include "ModelicaStrings.c"
/* MSL 4.0 has replaced the inclusion of win32_dirent.c with win32_dirent.h in ModelicaInternal.c, presumably as a part of
   the transistion to a new library ModelicaExternal.lib where win32_dirent.c is incorporated. However, until ModelicaExternal.lib
   is actually in place, we need win32_dirent.c from somewhere else and here is a good place.
TODO: remove when ModelicaExternal.lib is part of MSL
 */
#if defined(_WIN32_DIRENT_H_) && !defined(SUFFIX)
#include "win32_dirent.c"
#endif


#if ! defined(RootFinder_)
#   if ! defined(DynSimStruct) || defined(Matlab51) || defined(FMU_SOURCE_CODE_EXPORT)
#      define RootFinder_ 1     /* Root finder available in integration algorithm. */
#   else
#      define RootFinder_ 0
#   endif 
#endif

#if ! defined(EventIterate_)
#   define EventIterate_ 1     /* 1: Iterate for consistent conditions after event. 
                                  0: Propagate event next time. */
#endif

#if ! defined(TryLUFactorization_) 
#define TryLUFactorization_ 0 /* 0: Do not attempt LU-factorization (i.e. use QR-factorization)*/
#endif

#if ! defined(EquilibMatrix_)
#define EquilibMatrix_ 1 /* Row and column equilibrate matrix: Improves reliability at some cost */
#endif

#if ! defined(LargeEPS_)
#define LargeEPS_ 1e-4 /* An upper bound for the event epsilon */
#endif

#if ! defined(SlowIter_)
#define SlowIter_ 5 /* Number of iterations before attempting to increase event epsilon */
#endif

#if ! defined(SpuriousEvents_)
#   define SpuriousEvents_ 0    
#endif

#if ! defined(DAEsolver_)
#if defined(DYNExposeDAEResiduals_)
#   define DAEsolver_ 1
#else
#   define DAEsolver_ 0
#endif
#endif

#if !defined(DymolaAimForHighAccuracy_)
#define DymolaAimForHighAccuracy_ 1
#endif

#if !defined(DymolaNewJacobian_)
#define DymolaNewJacobian_ 1
#endif

#if !defined(DymolaAllowNewJacobianAnyway_)
#define DymolaAllowNewJacobianAnyway_ 1
#endif

#if !defined(DymolaNrStepTimers)
#define DymolaNrStepTimers 100000
#endif

#if !defined(DymolaNonLinearIterations_)
#define DymolaNonLinearIterations_ 0
#elif DymolaNonLinearIterations_>8
#error Cannot set DymolaNonLinearIterations_ above 8
#undef DymolaNonLinearIterations_
#define DymolaNonLinearIterations_ 8
#endif

#if 0 && DymolaPrecisionTiming_ && !defined(DymolaUninterruptedTimers_)
#define DymolaUninterruptedTimers_ 1
#endif

#if DymolaPrecisionTiming_ && !defined(DymolaUseRDTSC_) && defined(DYNGenerateTimers_)
#define DymolaUseRDTSC_  1
#endif

#ifndef DYNInitialGuessInterpolation_
#define DYNInitialGuessInterpolation_ 1
#endif
#ifndef DYNInitialGuessPolynomialUpdate_
#define DYNInitialGuessPolynomialUpdate_ 1
#endif
#ifndef DYNKeepCVodeFEvalNonlinearGuesses_
#define DYNKeepCVodeFEvalNonlinearGuesses_ 1
#endif
#ifndef DYNInitialGuessUpdate_
#define DYNInitialGuessUpdate_ 2
#endif

#ifndef DYNFullModelHomotopy_
#define DYNFullModelHomotopy_ 0
#endif

#ifndef DYNNonlinearImpreciseSolution_
#define DYNNonlinearImpreciseSolution_ 1
#endif

#include "dsutil.h"

#if !defined(DYMOLA_DSPACE) && !defined(RT)
#if DymolaUninterruptedTimers_ && defined(_MSC_VER) && (defined(_M_AMD64)||defined(_M_IX86))
#include "intrin.h"
#endif
void DymolaOnInterrupt(void) {
#if DymolaUninterruptedTimers_ && defined(_MSC_VER) && (defined(_M_AMD64)||defined(_M_IX86))
		_enable();
#elif DymolaUninterruptedTimers_ && defined(__GNUC__) && defined(i386)
		__asm__("sti");
#endif
}
int DymolaNoInterrupt(void) {
#if DymolaUninterruptedTimers_ && defined(_MSC_VER) && (defined(_M_AMD64)||defined(_M_IX86))
		_disable();
#elif DymolaUninterruptedTimers_ && defined(__GNUC__) && defined(i386)
		__asm__("cli");
#endif
		return 0;
}
#endif

DYMOLA_STATIC void GetDimensions2(int *nx_, int *nx2_, int *nu_, int *ny_, int *nw_, 
  int *np_, int*nsp, int *nrel2_, int *nrel_, int *ncons_, int *dae_);

#if defined(DYMOLA_DSPACE) && defined(__GNUC__)
#define ZERO_INITIALIZED = {0}
#else
#define ZERO_INITIALIZED /* = {0} */
#endif


#define DymolaStoreAuxiliaries_ (((struct BasicIDymosimStruct*)(DSE_STRUCT iuser_))->mDymolaStoreAuxiliaries)
      /* Auxiliary variables. */
	  static int DYNStrInit(struct DYNInstanceData*);
	  DYMOLA_STATIC void DYNSetAuxString(struct DYNInstanceData*did_,const char*,int i);
	  DYMOLA_STATIC void DYNSetAuxStringD(struct DYNInstanceData*did_, const char*, int i,int setDEventIfDifferent);
	  DYMOLA_STATIC void DYNSetAuxStringArray(struct DYNInstanceData*did_,struct StringArray,int i);
	  DYMOLA_STATIC void DYNSetAuxStringArrayD(struct DYNInstanceData*did_, struct StringArray, int i, int setDEventIfDifferent);
	  DYMOLA_STATIC const char*DYNGetAuxStr(struct DYNInstanceData*did_,int i);

      /* Variables to handle non-linear systems of equations. */
#define Qtol (did_->Qtol_var)

	  DYMOLA_STATIC int* QNLfunc;
	  DYMOLA_STATIC int* QNLjac;
	  DYMOLA_STATIC int *QNLcalls;
	  DYMOLA_STATIC int QNLmax;


#define PrintEvent (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mPrintEvent)
#define inJacobian_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mInJacobian)
#define solverHandleEq_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mSolverHandleEq)
#define continueSimulate_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mContinueSimulate)
#define dymolaParametersNr_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mParametersNr)
#define atDAEEvent_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mAtDAEEvent)
#define rememberOnlyAccepted_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mRememberOnlyAccepted)
#define usingCVodeGodess_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mUsingCVodeGodess)
#define blockUnblockSmoothCrossings_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mBlockUnblockSmoothCrossings)


      static int nx_, nx2_, nu_, ny_, nw_, np_, nsp_, nrel2_, nrel_, ncons_, dae_;

#define DYNTime *time
#define DYNTime2 (did_->time_var)
#define DYNIdemand2 (did_->idemand_var)
#define InitTime (did_->InitTime_var)
#define LastTime (did_->LastTime_var)
#define StepSize (did_->StepSize_var)
#define currentStepSize_ (did_->currentStepSize_var)
#define currentStepSizeRatio_ (did_->currentStepSizeRatio_var)
#define previousTime_ (did_->previousTime_var)
#define DYNhelp (did_->helpvar_vec)
#define DYNEvent (did_->Event_var)
#define initializationPhase_ (did_->initializationPhase_var)
#define Iter (did_->Iter_var)
#define MaxIter (did_->MaxIter_var)
#define EPS_ (did_->EPS_var)
#define FirstCross_ (did_->FirstCross_var)
#define FirstEvent (did_->FirstEvent_var)
#define ResetCounter_ (did_->ResetCounter_var)
#define DYNHReject (did_->HReject_var)

#define GlobalError_ (did_->GlobalError_var)
#define MixedFailFlag_ (did_->MixedFailFlag_var)
#define PerformIteration_ (did_->PerformIteration_var)

#define DymolaHomotopyLambda (did_->DymolaHomotopyLambda_var)
#define DymolaHomotopyLambdaFail (did_->DymolaHomotopyLambdaFail_var)
#define DymolaHomotopyLambdaDelta (did_->DymolaHomotopyLambdaDelta_var)
#define DymolaHomotopyUsed (did_->DymolaHomotopyUsed_var)
#define DymolaUserHomotopy (did_->DymolaUserHomotopy_var)

#define DymolaOneIteration_ (did_->DymolaOneIteration_var)
#define EqRemember1Time_ (did_->EqRemember1Time_var)
#define EqRemember2Time_ (did_->EqRemember2Time_var)
#define QPre_ (did_->QPre_vec)
#define RefPre_ (did_->RefPre_vec)
#define EqRemember1_ (did_->EqRemember1_vec)
#define EqRemember2_ (did_->EqRemember2_vec)
#define DYNAuxStr_ (did_->DYNAuxStrPtr_vec)
#define Aux_ (did_->Aux_vec)
#define QEvaluate_ (did_->QEvaluate_vec)
#define QEvaluateNew_ (did_->QEvaluateNew_vec)
#define QCheckIf_ (did_->QCheckIf_vec)
#define QTimed_ (did_->QTimed_vec)
#define Qenable_ (did_->Qenable_vec)
#define NextSampleTime_ (did_->NextSampleTime_vec)
#define NextSampleTimeNew_ (did_->NextSampleTimeNew_vec)
#define NextSampleAct_ (did_->NextSampleAct_vec)
#define NextSampleActNew_ (did_->NextSampleActNew_vec)
#define QL_ (did_->QL_vec)
#define QRel_ (did_->QRel_vec)
#define QM_ (did_->QM_vec)
#define Qn_ (did_->Qn_vec)
#define Qp_ (did_->Qp_vec)
#define Qscaled_ (did_->Qscaled_vec)
#define QZold_ (did_->QZold_vec)
#define oldQZ_ (did_->oldQZ_vec)
#define oldQZDummy_ (did_->oldQZDummy_vec)
#define oldQZ2_ (did_->oldQZ2_vec)
#define oldQZ3_ (did_->oldQZ3_vec)
#define QImd_ (did_->QImd_vec)
#define QIml_ (did_->QIml_vec)
#define Init_ (did_->Init_var)
#define AnyEvent_ (did_->AnyEvent_var)
#define AnyDEvent_ (did_->AnyDEvent_var)
#define AnyREvent_ (did_->AnyREvent_var)
#define AnyIEvent_ (did_->AnyIEvent_var)
#define AnyEventParameter_ (did_->AnyEventParameter_var)
#define NewParameters_ (did_->NewParameters_var)
#define dymolaEventsNr_ (did_->dymolaEventsNr_var)
#define EnforceWhen_ (did_->enforceWhen_var)
#define D_ (did_->discreteState_var)
#define XP_ (did_->pre_var)


#define EqRemAcc1Time_ (did_->eqRememberAccepted1Time)
#define EqRemAcc2Time_ (did_->eqRememberAccepted2Time)
#define EqRememberTemp_ (did_->EqRememberTemp_vec)
#define EqRemTempTime_ (did_->eqRememberTempTime)
#define EqRemTempTimeAcc_ (did_->eqRememberTempTimeAcc)

#define DYNAdvicePrinted_ (did_->nonlinearFailureAdvicePrinted)

#define DelayStruct_ (did_->del_vec)
#define delayID_ (did_->delayID_vec)

#define DYNMixedNiter_ (did_->mixedNiter_var)

#ifndef DYMOLA_TIMES
#define DYMOLA_TIMES
struct DymolaTimes {
	int num;
	double maxim;
	double minim;
	double total;
	int numAccum;
	double maximAccum;
	double minimAccum;
	double totalAccum;
	const char*name;
	int mask;
};
#endif

#if !defined(NCheckIf_)
#define NCheckIf_ 0
#endif
#if !defined(NGlobalHelp_)
#define NGlobalHelp_ 100
#endif
#if !defined(NGlobalHelpI_)
#define NGlobalHelpI_ 100
#endif
#if NGlobalHelp_ > 268435454
#define DYNLargeGlobalHelp NGlobalHelp_
#endif
#include "delay.h"
#ifndef MAX_EC
#define MAX_EC 10
#endif
#ifndef NSparse_
#define NSparse_ 0
#endif
#ifdef DYMOSIM
#define NUM_ITERATION_VARIABLES (1)
#else
#define NUM_ITERATION_VARIABLES (DAEsolver_ && NX2_ ? NX2_ : 1)
#endif

	  typedef struct DYNInstanceData {
		  struct BasicDDymosimStruct*basicD;
		  struct BasicIDymosimStruct*basicI;
		  int DymolaOneIteration_var; /* =2 first, =3 subsequent, 4 subsequent no iter, 5 subsequent without iteration, return !=0 => need more */
		  int HaveEventIterated_var;
		  int DymolaEventOptional_var;
		  int inCall_var;
		  delayStruct del_vec[SizeDelay_ ? SizeDelay_ : 1];
		  int delayID_vec[SizeDelay_ ? SizeDelay_ : 1];
#ifdef DYNLargeGlobalHelp
		  double *helpvar_vec;
#else
		  double helpvar_vec[NGlobalHelp_+1];
#endif
		  int helpvari_vec[NGlobalHelpI_+1];
		  double time_var;
		  double InitTime_var;
		  double LastTime_var;
		  double StepSize_var;
		  double currentStepSize_var;
		  double currentStepSizeRatio_var;
		  double currentStepSizeRatio2_var; /* Unused */
		  double previousTime_var;

		  Dymola_bool Event_var;
	      int initializationPhase_var;
	      int Iter_var, MaxIter_var;
          double EPS_var;
          int FirstCross_var;
          Dymola_bool FirstEvent_var;
	      int ResetCounter_var;
		  int HReject_var;

		  int GlobalError_var;
		  int MixedFailFlag_var;
		  Dymola_bool PerformIteration_var;

		  double DymolaHomotopyLambda_var;
		  double DymolaHomotopyLambdaFail_var;
		  double DymolaHomotopyLambdaDelta_var;
		  int DymolaHomotopyUsed_var;
		  int DymolaUserHomotopy_var;

		  double TimeStartOfSimulation_var;

		  double EqRemember1Time_var;
		  double EqRemember2Time_var;
		  struct DymolaTimes DymolaTimerStructs_vec[
#ifdef NrDymolaTimers_
			  NrDymolaTimers_ ? NrDymolaTimers_ : 1
#else
			  1
#endif
		  ];
		  int DymolaTimerStructsLen_var;
		  double DymolaStartTimers_vec[
#ifdef NrDymolaTimers_
NrDymolaTimers_ ? NrDymolaTimers_ : 1
#else
	1
#endif
			];
            double DymolaTimeZero_vec[DymolaNrStepTimers];
            int DymolaTimeZeroLength_var;
			int DymolaTimecounter_var;
		  double QPre_vec[SizePre_?SizePre_:1];
		  double RefPre_vec[SizePre_?SizePre_:1];
		  double EqRemember1_vec[SizeEq_?SizeEq_:1];
		  double EqRemember2_vec[SizeEq_?SizeEq_:1];
		  double Aux_vec[MAXAux_?MAXAux_:1];
		  Dymola_bool QEvaluate_vec[NWhen_+1];
		  Dymola_bool QEvaluateNew_vec[NWhen_+1]; 
		  double QCheckIf_vec[NCheckIf_+1];
		  Dymola_bool QTimed_vec[NTim_+1];
		  Dymola_bool Qenable_vec[NRel_+1];
		  int oldReset_var;
		  double NextSampleTime_vec[NSamp_+1];
	      double NextSampleTimeNew_vec[NSamp_+1];
	      Dymola_bool NextSampleAct_vec[NSamp_+1];
	      Dymola_bool NextSampleActNew_vec[NSamp_+1];
		  Dymola_bool QL_vec[NRel_+1];
	      double QRel_vec[NRel_+1];
	      double QM_vec[NRel_+1];
	      double Qn_vec[NRel_+1];
	      double Qp_vec[NRel_+1];
	      double Qscaled_vec[NRel_+1];
	      double QZold_vec[2*NRel_+1];
	      double oldQZ_vec[2*NRel_+1];
	      double oldQZDummy_vec[2*NRel_+1];
	      double oldQZ2_vec[2*NRel_+1];
		  double oldQZ3_vec[2*NRel_+1];
		  Dymola_bool Init_var, AnyEvent_var, AnyDEvent_var, AnyREvent_var, AnyIEvent_var, AnyEventParameter_var, NewParameters_var;
		  struct ExternalTable_ externalTable_vec[
#ifdef NExternalObject_ 
			NExternalObject_+1
#else
			  1
#endif
		  ];
		  int dymolaParametersNrOld_;
	      int dymolaEventsNr_var;
		  int enforceWhen_var;

	      double * QJacobian_var;
	      double * QBJacobian_var;
	      double * QCJacobian_var;
	      double * QDJacobian_var;
		  int NewDir_var;
		  const double * QXDir_var;
		  const double * QUDir_var;
		  double * QDDir_var;
		  double * QYDir_var;
	      int QJacobianN_var;
	      int QJacobianNU_var;
	      int QJacobianNY_var;
	      double * QJacobianSparse_var;
	      int* QJacobianSparseR_var ;
	      int* QJacobianSparseC_var ;
	      int QJacobianNZ_var;
		  int QSparseABCD_var;
		  /* Adjoints: */
		  double* QFR_var;
		  double* QWR_var;
		  double* QHelpR_var;
		  double* QUR_var;
		  double* QYR_var;
		  double* QXR_var;
		  double* QPR_var;
		  double* QAux_var;

		  double QImd_vec[
#ifdef NI_
	NI_*(2*NI_+5)+NX_*(4+3*NI_)+	
#endif
1
];
		  int QIml_vec[
#ifdef NI_
	5*NI_+
#endif
	1
];
		  char DYNAuxStrBuff_vec[
#if defined(MAXAuxStr_) && MAXAuxStr_>0
			MAXAuxStr_
#else
		  1
#endif
		  *
#if defined(MAXAuxStrLen_) && MAXAuxStrLen_>10
			MAXAuxStrLen_
#else
		  10
#endif
			  ];
			char* DYNAuxStrPtr_vec[
#if defined(MAXAuxStr_) && MAXAuxStr_>0
			MAXAuxStr_
#else
		  1
#endif
			];
			int currentIteration_var;
			double discreteState_var[ND_+1];
			double pre_var[NXP_+1];
			double xInitial_var[NInitial_+1];
			double xInitial_var2[NInitial_+1];
			double decoupleTime_var;
			Dymola_bool sectioncondition_var;
			double Qtol_var;

			double eqRememberAccepted1Time;
			double eqRememberAccepted2Time;
			double EqRememberTemp_vec[SizeEq_ ? SizeEq_ : 1];
			double eqRememberTempTime;
			double eqRememberTempTimeAcc;
			int hasStoredInTemp;
			int eqRememberReplaceOldDynamics;
			int eqRememberReplaceOldAccepted;

			int nonlinearFailureAdvicePrinted;
			char fmiResourceLocation[1024];

			int DymolaHomotopyExponential_var;

			int mixedNiter_var;

			jmp_buf exception_buffer[MAX_EC];
			int exception_count;

			int idemand_var;

			void* sparse_data[NSparse_ ? NSparse_ : 1];

			double dae_iteration_variable_nominals[NUM_ITERATION_VARIABLES];
#if NAR_ > 0
			struct RealArray UR_[NAR_];
#endif
#if NAI_ > 0
			struct IntegerArray UI_[NAI_];
#endif
#if NAS_ > 0
			struct IntegerArray US_[NAS_];
#endif
	  } DYNInstanceData;
	  /* Temporary ones: */

#define sectioncondition (did_->sectioncondition_var)
	  DYMOLA_STATIC int isModelicaEvent(void);
	  DYMOLA_STATIC int* GlobalErrorPointer(void);

struct BasicDDymosimStruct*getBasicDDymosimStructNew(struct DYNInstanceData*did) {
	if (did && did->basicD) return did->basicD;
	return getBasicDDymosimStruct();
}
#define triggerStepEvent_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mTriggerStepEvent)
#ifndef FindEvent_
#define DYNFindEvent (((struct BasicIDymosimStruct*)  (DSE_STRUCT iuser_))->mFindEvent)
#else
#define DYNFindEvent FindEvent_
#if 0
#undef FindEvent_
#define FindEvent_ 1
#endif
#endif

#define Dymola_Simulation_TriggerResultSnapshot_ (((struct BasicIDymosimStruct*) (DSE_STRUCT iuser_))->mTriggerResultSnapshot)
	  
#define DYMResourceQuote2(fmin) #fmin
#define DYMResourceQuote(fmin) DYMResourceQuote2(fmin)

#if defined(DYN_MULTINSTANCE)
DYMOLA_STATIC size_t dyn_allowMultipleInstances=sizeof(struct DYNInstanceData);
#else
DYMOLA_STATIC size_t dyn_allowMultipleInstances=0;
#endif
DYMOLA_STATIC size_t dyn_instanceDataSize = sizeof(struct DYNInstanceData);

#if defined(_WIN32) && (defined(_MSC_VER) || defined(DYN_MULTINSTANCE)) || defined(__MINGW32__)
#include <windows.h>
#if defined(DYN_MULTINSTANCE)
static DWORD dsTlsDynInstance=0;
#endif
#if !defined(FMU_SOURCE_CODE_EXPORT) && defined(FMI_MODULE_NAME)
#ifdef USE_DELAY_LOAD
#include <delayimp.h>
#endif
#if defined(DYN_MULTINSTANCE)
DYMOLA_STATIC void EnsureMarkFree(struct DYN_ThreadData*threadData);
#endif
BOOL __stdcall DllMain(HINSTANCE hinstDLL,  /* DLL module handle*/
    DWORD fdwReason,              /* reason called*/
    LPVOID lpvReserved)           /* reserved*/
{ 
    LPVOID lpvData; 
    BOOL fIgnore;
#ifdef USE_DELAY_LOAD
	extern const char* dllLibraryPath[];
	size_t j;
#endif
 
    switch (fdwReason) 
    { 
        /* The DLL is loading due to process */
        /* initialization or a call to LoadLibrary. */
 
        case DLL_PROCESS_ATTACH: 
 
            /* Allocate a TLS index.*/
 #if defined(DYN_MULTINSTANCE)
            if ((dsTlsDynInstance = TlsAlloc()) == 0xFFFFFFFF) 
                return FALSE; 
#endif
           /* No break: Initialize the index for first thread.*/
 
        /* The attached process creates a new thread. */
 
        case DLL_THREAD_ATTACH: 
 
            /* Initialize the TLS index for this thread.*/

 
            break; 
 
        /* The thread of the attached process terminates.*/
 
        case DLL_THREAD_DETACH: 
 
            /* Release the allocated memory for this thread.*/
#if defined(DYN_MULTINSTANCE) 
            lpvData = TlsGetValue(dsTlsDynInstance); 
            if (lpvData != NULL) {
				EnsureMarkFree((struct DYN_ThreadData*)(lpvData));
                LocalFree((HLOCAL) lpvData); 
			}
			lpvData = NULL;
			TlsSetValue(dsTlsDynInstance, lpvData);
#endif
            break; 
 
        /* DLL unload due to process termination or FreeLibrary.  */
 
        case DLL_PROCESS_DETACH: 
 
            /* Release the allocated memory for this thread. */
 #if defined(DYN_MULTINSTANCE)
            lpvData = TlsGetValue(dsTlsDynInstance); 
            if (lpvData != NULL) {
				EnsureMarkFree((struct DYN_ThreadData*)(lpvData));
                LocalFree((HLOCAL) lpvData); 
			}
			lpvData = NULL;
			TlsSetValue(dsTlsDynInstance, lpvData);
            /* Release the TLS index. */
			TlsFree(dsTlsDynInstance); 
#endif
#ifdef USE_DELAY_LOAD
			for (j = 0; dllLibraryPath && dllLibraryPath[j]; ++j) {
				LPCSTR toUnLoad = dllLibraryPath[j];
				if (toUnLoad[0] == '.' && toUnLoad[1] == '\\') {
					/* Special handling of local DLLs */
					toUnLoad += 2;
				}
				__FUnloadDelayLoadedDLL2(toUnLoad);
			}
#endif
#ifndef __MINGW32__
			freeClocale();
#endif
            break; 
 
        default: 
            break; 
    } 
 
    return TRUE; 
    UNREFERENCED_PARAMETER(hinstDLL); 
    UNREFERENCED_PARAMETER(lpvReserved); 
}
#endif /*!defined(FMU_SOURCE_CODE_EXPORT)*/
#if defined(DYN_MULTINSTANCE)
static struct DYN_ThreadData* DYN_GetThreadData() {
	LPVOID lpvData; 
	lpvData = TlsGetValue(dsTlsDynInstance); 
    if (lpvData == NULL) {
		/* This is a critical region, but since using Tls it is already guarded */
		lpvData = LocalAlloc(LPTR, sizeof(struct DYN_ThreadData)); /* LPTR means fixed and zero-initialized */
		if (lpvData) {
			TlsSetValue(dsTlsDynInstance, lpvData);
		}
	}
	return (struct DYN_ThreadData*)(lpvData);
}
#endif /*defined(DYN_MULTINSTANCE)*/
#endif
#if defined(DYN_MULTINSTANCE) && !(defined(_WIN32))
#if defined(__GNUC__) || DYN_COMPILER_SUPPORTS__THREAD
static __thread struct DYN_ThreadData dyn_threadData={0};
static struct DYN_ThreadData* DYN_GetThreadData() {
	return &dyn_threadData;
}
#else
#error Multiple instance only supported for Windows and Gcc so far
#endif
#endif

#if defined(RT) || defined(NRT)
#if defined(DYN_MULTINSTANCE)
LIBDS_API struct BasicIDymosimStruct*getBasicIDymosimStruct() {
	return DYN_GetThreadData()->did->basicI;
}
LIBDS_API struct BasicDDymosimStruct*getBasicDDymosimStruct() {
	return DYN_GetThreadData()->did->basicD;
}
LIBDS_API void setBasicStruct(double*d,int*i) {
	if (!DYN_GetThreadData()->did) return;
	DYN_GetThreadData()->did->basicI=(struct BasicIDymosimStruct*)(i);
	DYN_GetThreadData()->did->basicD=(struct BasicDDymosimStruct*)(d);
}
#else
static struct BasicIDymosimStruct*basicI=0;
static struct BasicDDymosimStruct*basicD=0;
LIBDS_API struct BasicIDymosimStruct*getBasicIDymosimStruct() {
	return basicI;
}
LIBDS_API struct BasicDDymosimStruct*getBasicDDymosimStruct() {
	return basicD;
}
LIBDS_API void setBasicStruct(double*d,int*i) {
	basicI=(struct BasicIDymosimStruct*)(i);
	basicD=(struct BasicDDymosimStruct*)(d);
}
#endif
#endif

static const char*dymosimResourceLocation=0;
#if defined(FMI_MODULE_NAME) && !defined(NO_EXTERNAL_DLL)
DYMOLA_STATIC const char* dymosimFMIPath() {
	static char extraPath[1000];
#if defined(NO_FILE) || defined (DYM2CCUR)
	extraPath[0] = 0;
#elif (defined(_WIN32) && defined(_MSC_VER)) || defined(__MINGW32__)
	HMODULE hMySelf=0;
	hMySelf=GetModuleHandleA(DYMResourceQuote(FMI_MODULE_NAME));
	extraPath[0] = 0;
	GetModuleFileNameA(hMySelf, extraPath, sizeof(extraPath)-1);
#elif (defined(__unix__) || defined(__linux__) || defined(__APPLE_CC__)) && defined(__GNUC__)
   Dl_info dl_info;
   dladdr(dymosimFMIPath, &dl_info);
   extraPath[0] = 0;
   strncpy(extraPath, dl_info.dli_fname, sizeof(extraPath)-1);
#endif
   extraPath[sizeof(extraPath)-1] = 0;
   return extraPath;
}
#endif
static char dymosimResourceExtraPath[1000];
#if !defined(NO_EXTERNAL_DLL)
#if (defined(_WIN32) && defined(_MSC_VER)) || defined(__MINGW32__) && defined(FMI_MODULE_NAME)
/* Fallback - see below */
DYMOLA_STATIC HINSTANCE GetMyModuleHandle() {
	MEMORY_BASIC_INFORMATION base; 
	VirtualQuery(GetMyModuleHandle, &base, sizeof(base)); 
	return (HINSTANCE)(base.AllocationBase); 
}
#endif
#endif

DYMOLA_STATIC const char* dymosimResources2(struct DYNInstanceData*did_) {
	static int first=1;
#if defined(MatlabR13) && !defined(xMOD)
	return "DymosimResources/";
#endif
	if (did_ && did_->fmiResourceLocation[0] != '\0') {
		return did_->fmiResourceLocation;
	}
#if !defined(NO_EXTERNAL_DLL) || defined(DYM2CCUR)
	if (first && !dymosimResourceLocation) {

		char*last;
		int twice = 0;
#if (defined(_WIN32) && defined(_MSC_VER)) || defined(__MINGW32__)
		HMODULE hMySelf = 0;
		const char* separator = "\\";
#ifdef FMI_MODULE_NAME
		hMySelf = GetModuleHandleA(DYMResourceQuote(FMI_MODULE_NAME));
		twice = 1;
		if (hMySelf == 0) {
			/* Oh, likely an 8+3 filename mess, try another variant */
			hMySelf = GetMyModuleHandle();
	    }
#else
		hMySelf=GetModuleHandleA(0);
#endif /* FMI_MODULE_NAME */
		dymosimResourceExtraPath[0] = 0;
		if (GetModuleFileNameA(hMySelf, dymosimResourceExtraPath, sizeof(dymosimResourceExtraPath)/sizeof(*dymosimResourceExtraPath))!=0) {

#elif defined(__GNUC__)
		{
			const char* separator = "/";
#if defined(UNIX) && defined(FMI_MODULE_NAME)
			/* Linux gcc and FMU so file */
			twice = 1;

			Dl_info dli;
			if (dladdr(__builtin_return_address(0), &dli) != 0 && dli.dli_fname != NULL) {
				strcpy(dymosimResourceExtraPath, dli.dli_fname);
			} else {
				DymosimMessage("Error: dladdr failed to fetch location of this FMU so file.");
			}
#else  /* defined(UNIX) && defined(FMI_MODULE_NAME) */
			/* resources for non-FMUs not supported on Linux, neglect */
#endif /* defined(UNIX) && defined(FMI_MODULE_NAME) */

#else  /* defined(__GNUC__) ... */
			/* neither Windows nor gcc, neglect */
		{
					const char* separator = "/";
#endif /* defined(__GNUC__) ... */
#ifdef __MINGW32__
			/* Fetch path from explcitly stored file created during translation.
			   This special treatment is a consequence of the fact that MinGW gcc does not support delay loading. */
			{
				FILE* f;
				strcat(dymosimResourceExtraPath, ".originalPath.txt");
				f = fopen(dymosimResourceExtraPath, "r");
				if (f == NULL) {
					DymosimMessage("Error: failed to open file with original path for this FMU");
				}
				else {
					char format[32];
					snprintf(format, sizeof(format), "%%%us", sizeof(dymosimResourceExtraPath) / sizeof(*dymosimResourceExtraPath) - 1);
					fscanf(f, format, dymosimResourceExtraPath);
					fclose(f);
				}
			}
#endif /* __MINGW32__ */
			dymosimResourceExtraPath[sizeof(dymosimResourceExtraPath)/sizeof(*dymosimResourceExtraPath)-1]=0; /* /Make sure it is NUL-terminated */
			last=strrchr(dymosimResourceExtraPath,'\\');
			if (last==0) last=strrchr(dymosimResourceExtraPath,'/');
			if (last && twice) {
				last[0]=0;
				last=strrchr(dymosimResourceExtraPath,'\\');
				if (last==0) last=strrchr(dymosimResourceExtraPath,'/');
				if (last) {
					last[0]=0;
					last=strrchr(dymosimResourceExtraPath,'\\');
					if (last==0) last=strrchr(dymosimResourceExtraPath,'/');
				}
			}
			if (last) last[1]=0;
			if (twice) {
				strcat(dymosimResourceExtraPath,"resources");
				strcat(dymosimResourceExtraPath,separator);
			} else {
				strcat(dymosimResourceExtraPath,"DymosimResources");
				strcat(dymosimResourceExtraPath,separator);
			}	
			dymosimResourceLocation= dymosimResourceExtraPath;
		}
		first=0;
	}
#endif
	return dymosimResourceLocation ? dymosimResourceLocation : "DymosimResources/";
}
DYMOLA_STATIC const char* dymosimResources() {
	return dymosimResources2(0);
}
#undef homotopy
#define homotopy(a,b) (DYNhomotopy(a,b,did_))
DYMOLA_STATIC double DYNhomotopy(double actual, double simple,struct DYNInstanceData*did_) {
	did_->DymolaHomotopyUsed_var = 1;
	if (did_->DymolaHomotopyExponential_var) {
		double exp_lambda = 0.0;
		if (DymolaHomotopyLambda <= 0.0) return simple;
		if (DymolaHomotopyLambda >= 1.0) return actual;
		exp_lambda = pow(0.1, 10.0*DymolaHomotopyLambda);
		return simple * exp_lambda + actual * (1.0 - exp_lambda);
	} else 
		return simple*(1-DymolaHomotopyLambda)+actual*DymolaHomotopyLambda;
}
DYMOLA_STATIC void dymosimSetResources2(struct DYNInstanceData*did_, const char*s) {
	char* rps = 0;
	size_t sz = 0;
	if (did_) {
		rps = did_->fmiResourceLocation;
		sz = sizeof(did_->fmiResourceLocation);
		rps[0] = '\0';
	}
	else {
		rps = dymosimResourceExtraPath;
		sz = sizeof(dymosimResourceExtraPath);
		rps[0] = '\0';
	}
	if (s && s[0] != '\0' && strlen(s) < sz - 1) {
		{
			char* rp = rps;
			const char* sp = s;
			const char* cp = 0;
			while ((cp = strchr(sp, '%'))) {
				ptrdiff_t len = cp - sp;
				strncpy(rp, sp, len);
				rp += len;
				sp = cp + 1;
				if (sp[0] && sp[1]) {
					char c[3] = { sp[0] , sp[1], 0 };
					long num = strtol(c, 0, 16);
					if (num) {
						*rp++ = (char)num;
					}
					sp += 2;
				}
			}
			strncpy(rp, sp, sz - (sp - s) - 1);
		}
		rps[sz - 2] = '\0';
		if (rps[strlen(rps) - 1] != '/' && rps[strlen(rps) - 1] != '\\') strcat(rps, "/");
		if (!did_)
			dymosimResourceLocation = rps;
	}
	else {
		if (!did_)
			dymosimResourceLocation = 0;
	}
}
DYMOLA_STATIC void dymosimSetResources(const char*s) {
	dymosimSetResources2(0, s);
}
static DYN_UNUSED_ATTR void handleevent4funcMinor(struct DYNInstanceData*did_,const char*rele,const char*sube,int index, int PrintEvent2,int ltz,int invres) {
	handleevent4(rele,sube,&QL_[index], &Qp_[index-1],&Qn_[index-1],EPS_*Qscaled_[index-1],QRel_[index-1],Init_, FirstEvent?-PrintEvent2:PrintEvent2,&AnyEvent_,ltz,invres,Qenable_[index],LargeEPS_*Qscaled_[index-1],Iter>SlowIter_,index);
}
static DYN_UNUSED_ATTR void handleevent4func(struct DYNInstanceData*did_,const char*rele,const char*sube,int index, int PrintEvent2,int ltz,int invres) {
	handleevent5(rele,sube,&QL_[index], &Qp_[index-1],&Qn_[index-1],EPS_*Qscaled_[index-1],QRel_[index-1],Init_, FirstEvent?-PrintEvent2:PrintEvent2,&AnyEvent_,&AnyDEvent_,ltz,invres,Qenable_[index],LargeEPS_*Qscaled_[index-1],Iter>SlowIter_,index);
}
static DYN_UNUSED_ATTR Dymola_bool handleevent4Sfunc(struct DYNInstanceData*did_,const char*rele,const char*sube,double*QZV,int index, int idemand,double Time,int PrintEvent2,int ltz,int invres, int usingCVodeGodess) {
	Dymola_bool AE=0;
	Dymola_bool AE2 = 0;
	Dymola_bool res;
	res=handleevent6S(rele,sube,
		&QL_[index],&Qp_[index-1],&Qn_[index-1],EPS_,QRel_[index-1],&QZV[2*(index)-2],Init_,DYNEvent,idemand,FirstEvent?-PrintEvent2:PrintEvent2,&AE,&AE2,ltz,invres,&Qenable_[index],LargeEPS_,Iter>SlowIter_,Time, Iter==1, index, usingCVodeGodess);
	if (AE) {AnyEvent_=1;AnyIEvent_=1;}
	if (AE2) {AnyDEvent_ = 1;}
	return res;
}
static DYN_UNUSED_ATTR int DYNIncrementFExternal() {
	AssertModelicaF(
#ifdef NSizeOfFunctionStack
		FunctionExternalContext_<NSizeOfFunctionStack
#else
	    0
#endif
		,"", "Out of stack for external objects in functions; see documentation to change.");

	return FunctionExternalContext_++;
}
#if defined(DYMOSIM)
LIBDS_API double PositiveImpulse(int, double, double,int);
LIBDS_API double SymmetricImpulse(int, double, int);
LIBDS_API double DiracImpulse(int, double, double, int);
LIBDS_API double DiracIntegral(int, double, double , int);
LIBDS_API double HeaviSide(int, double, double, int);
static void InitI(struct DYNInstanceData* did_,int,int);
#endif
#if (defined(DYMOSIM) || (defined(BUILDFMU) && !defined(FMU_SOURCE_CODE_EXPORT))) && !defined(LIBDS_DLL) && !defined(DYMOLA_AS_MKMK_MODULE)
LIBDS_API void* DynSuperLUMTInit(int* const, const int, const int, const int, double*, int*, int*);
LIBDS_API void DynSuperLUMTSolve(void* const, double* const, int* const, const double, const int, const char* const, int* const);
LIBDS_API void DynSuperLUMTFree(void* const);
#endif
#if !defined(DynSimStruct) && defined(DYMOSIM)
#include "dlldata.h"
#endif
#include "dymdstypes.h"
static DYN_UNUSED_ATTR double JumpImpulseCode(int c, double newValue, const char* ys, double* v, int impulseNr) {
#if !defined(DynSimStruct) && defined(DYMOSIM)
	if (*v != newValue && (c || dymosimGetThreadData()->impulseStruct.Dymola_activateImpulses)) {
		if (c && dymosimGetThreadData()->impulseStruct.typeImpulse[impulseNr] != DiracDeActive) {
			dymosimGetThreadData()->impulseStruct.typeImpulse[impulseNr] = DiracDeActive;
			if (!(getBasicIDymosimStruct()->mPrintEvent & (1 << 10))) {
				char buf[DYNDiagnosticBuf];
				sprintf(buf, "\nIntroducing impulse to handle differentiated expression that was not continuous:\n " DYNDiagnosticStr "\n Value jumped from %g to %g.\n", ys, *v, newValue);
				DymosimMessage(buf);
			}
		}
		return DiracImpulse(1, newValue - *v, 0, impulseNr);
	}
	dymosimGetThreadData()->impulseStruct.typeImpulse[impulseNr] = NoImpulse;
	*v = newValue;
#else
	CheckContinuousIfCode(c, ys, v, newValue);
#endif
	return 0;
}
#define JumpImpulse(newval,ys,ifId,impulseId) JumpImpulseCode(DYNEvent&&Iter>0&&!Init_&&DYNTime!=InitTime, newval, ys, QCheckIf_+ifId, impulseId)
#if defined(DYMOLA_DSPACE) || defined(NO_FILE) || defined(FMU_SOURCE_CODE_EXPORT) || defined(LABCAR)
static DYN_UNUSED_ATTR void DymolaStartTimerFunction(struct DYNInstanceData*did_, int x, const char*n, int p) { ; }
static DYN_UNUSED_ATTR void DymolaEndTimerFunction(struct DYNInstanceData*did_, int x) { ; }
#else
static DYN_UNUSED_ATTR void DymolaStartTimerFunction(struct DYNInstanceData*did_, int x, const char*n, int p) {
	DymolaTimerCounterSince(did_->DymolaStartTimers_vec + x, 0);
	did_->DymolaTimerStructs_vec[x].mask |= (3 + 4 * p);
	did_->DymolaTimerStructs_vec[x].name = n;
}
static DYN_UNUSED_ATTR void DymolaEndTimerFunction(struct DYNInstanceData*did_, int x) {
	if (did_->DymolaTimerStructs_vec[x].mask & 2) {
		double*time;
		time = &DYNTime2;
		DymolaEndTimerName(x, did_->DymolaTimerStructs_vec[x].name);
	}
}
#endif

#if defined(_MSC_VER)
#if !DymolaGlobalOptimizations_
/* Visual C++ does not work well for large models with global optimization.
   Remove at your own risk */
#pragma optimize( "g", off )
#elif DymolaGlobalOptimizations_ == 2
#pragma optimize( "g", on )
#endif
#endif

DYMOLA_STATIC void DYNInterpolateInputs(double* U_, double t, struct DYNInstanceData* did_);

#if defined(DS_EMBEDDED)
#include "dsembedded.c"
#endif
#if defined(DYNCALLFMUREINIT)
DYMOLA_STATIC void DelayedUpdateReinit(double*time, double* X_, double* W_,double* U_,double* Y_, struct DYNInstanceData*did_);
#endif
DYMOLA_STATIC void equations_(int *idemand_, int *icall_, 
      double *time, double X_[], double XD_[], double U_[], \
      double DP_[], int IP_[], Dymola_bool LP_[], \
      double F_[], double Y_[], double W_[], double QZ_[], 
      double duser_[], int iuser_[], void*cuser_[], struct DYNInstanceData* did_ DYNOPTArgCSD,
      int *QiErr) 
{   
/* End dsblock1.c */
