/* line below added during FMU creation */ 
#include "dlldata_impl.h"     
#ifdef _MSC_VER
#include "windows.h"
#ifndef NO_EXTERNAL_DLL
#include "shlwapi.h"
#endif
#endif
#ifndef DYMOLA_STATIC_IMPORT
#define DYMOLA_STATIC_IMPORT DYMOLA_STATIC
#endif
#ifndef FMU_SKIP_MODEL_EXCHANGE

/* The generic implementation of the FMI ME interface. */
/* need to include first so that correct files are included */
#include "conf.h"
#include "util.h"
#include "result.h"
#include "memdebug.h"

#include "fmiFunctions_fwd.h"

#include "dlldata.h"
#include "dymosim.h"

#include <string.h>
#include <assert.h>


/* platform specifics */
#if defined(_MSC_VER) && _MSC_VER >= 1400
/* avoid warnings from Visual Studio */
#define strncpy(dest, src, len) strncpy_s(dest, (len) + 1, src, len)
#else /* defined(_MSC_VER) && _MSC_VER >= 1400 */
/* for gcc */
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif /* defined(_MSC_VER) && _MSC_VER >= 1400 */

/* ----------------- global variables ----------------- */

/* This has 2 purposes:
   1. Detect attempts to create mutliple instances of same FMU.
   2. Direct logging from libds code to FMI logger, when providing FMU with source code. 
*/

/* when compiling as a single complilation unit, this is already defined */
#ifndef FMU_SOURCE_SINGLE_UNIT
extern Component* globalComponent;
extern Component* globalComponent2;
#endif

extern int QJacobianDefined_;


/* ----------------- macros ----------------- */

#define ME_RESULT_SAMPLE(atEvent)                                         \
if (comp->storeResult == FMITrue) {                                       \
	if(result_sample(comp, atEvent)){                                     \
		util_logger(comp, comp->instanceName, FMIFatal, loggUndef,        \
		"%s: Error when sampling result file, out of memory\n", label);   \
		return FMIFatal;                                                  \
	}                                                                     \
}

/* ----------------- external function declarations ----------------- */
struct DYNInstanceData;
DYMOLA_STATIC int GetDymolaOneIteration(struct DYNInstanceData*);
DYMOLA_STATIC void SetDymolaOneIteration(struct DYNInstanceData*, int val);
extern void declareNew2_(double*, double*, double*, double*, double*, double*, double*, void**,int*, int, struct DeclarePhase*);
DYMOLA_STATIC size_t DYNGetMaxAuxStrLen();


/* ----------------- local function declarations ----------------- */

FMIStatus initializeModel(FMIComponent c, FMIBoolean toleranceControlled, FMIReal relativeTolerance, FMIBoolean complete);

/* ----------------- function definitions ----------------- */

/* For 2.0 this is replaced by a common function named fmiGetTypesPlatform_. */
#if (FMI_VERSION < FMI_2)
DYMOLA_STATIC const char* fmiGetModelTypesPlatform_()
{
	return fmiModelTypesPlatform;
}
#endif

/* ---------------------------------------------------------------------- */
#ifndef FMU_SOURCE_SINGLE_UNIT
extern char* GUIDString;
#endif

#ifndef FMU_SOURCE_SINGLE_UNIT
	extern unsigned int FMIClockValueReferences_[];
	extern unsigned int FMIClockFirstValueReferences_[];
#endif

#endif /* FMU_SKIP_MODEL_EXCHANGE */

 struct DYNInstanceDataMinimal {
		  struct BasicDDymosimStruct*basicD;
		  struct BasicIDymosimStruct*basicI;
 };
extern size_t dyn_allowMultipleInstances;
extern size_t dyn_instanceDataSize;
DYMOLA_STATIC void DYNInitializeDid(struct DYNInstanceData*did_);
DYMOLA_STATIC int fmiUser_Instantiate();
DYMOLA_STATIC int fmiUser_Initialize();
DYMOLA_STATIC int fmiUser_Terminate();
DYMOLA_STATIC void fmiUser_Free();
#if (FMI_VERSION >= FMI_2)
DYMOLA_STATIC void dymosimSetResources2(struct DYNInstanceData*did_, const char*s);
#endif
#if (FMI_VERSION >= FMI_3)
FMIComponent fmi3InstantiateModelExchange_(fmi3String instanceName,
								  fmi3String fmuGUID,
								  fmi3String fmuResourceLocation,
								  fmi3Boolean visible,
								  fmi3Boolean loggingOn,
								  fmi3InstanceEnvironment instanceEnvironment,
								  fmi3LogMessageCallback logMessage)

#elif (FMI_VERSION >= FMI_2)
FMIComponent fmiInstantiateModel_(FMIString instanceName,
								  FMIString fmuGUID,
								  FMIString fmuResourceLocation,
								  const FMICallbackFunctions* functions,
								  FMIBoolean visible,
								  FMIBoolean loggingOn)
#else
FMIComponent fmiInstantiateModel_(FMIString instanceName,
								  FMIString fmuGUID,
								  fmiMECallbackFunctions functions,
								  FMIBoolean loggingOn)
#endif 
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3InstantiateModelExchange";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2Instantiate";
#else
	static FMIString label = "fmiInstantiateModel";
#endif
	Component* comp;
	size_t i;
	int QiErr = 0;
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
#if (FMI_VERSION == FMI_2)
	extern const int fmiClockedStates;
#endif
#if (FMI_VERSION >= FMI_3)
	/*Skip*/
	JacobianData* jacData;
#elif (FMI_VERSION >= FMI_2)
	const FMICallbackFunctions* funcs = functions;
	JacobianData* jacData;
#else
	fmiMECallbackFunctions* funcs = &functions;
#endif
	if (!dyn_allowMultipleInstances && globalComponent != NULL) {
		util_logger(globalComponent, instanceName != NULL ? instanceName : globalComponent->instanceName,
			FMIFatal, loggUndef, "%s: multiple instances within same process not supported", label);
		return NULL;
	}

#ifdef MEMLEAK_DEBUG
	{
		memdebug_setup();
	}
#endif
	if(fmiUser_Instantiate())
		return NULL;

	comp = (Component*) ALLOCATE_MEMORY(1 , sizeof(Component));
	if (comp == NULL) {
		goto fail;
	}
	if (dyn_allowMultipleInstances) {
		comp->did = (struct DYNInstanceData*)ALLOCATE_MEMORY(1, dyn_allowMultipleInstances);
		if (comp->did == NULL) {
			goto fail;
		}
		DYNInitializeDid(comp->did);
	} else {
		comp->did = 0;
	}
#if (FMI_VERSION >= FMI_2)
	{
		char offset = 0;
		if (fmuResourceLocation != 0 && strncmp(fmuResourceLocation, "file:/", 6) == 0) {
#ifdef _WIN32
		/* on Windows the leading '/' is omitted in file paths */
		offset += 6;
#else
		offset += 5;
#endif /* _WIN32 */
			if (fmuResourceLocation != 0 && strncmp(fmuResourceLocation + 6, "//", 2) == 0) {
				/* handle case file:/// */
				offset += 2;
			}
		}
#if (FMI_VERSION < FMI_3)
		/*in Fmi 3 the path is no longer required to start with file:/, 0 offset is OK*/
		if(offset){ 
#endif
		dymosimSetResources2(comp->did, fmuResourceLocation + offset);
		/* Note comp->did may be null, that is handled. */
#if (FMI_VERSION < FMI_3)
		}
#endif
	}
#endif /* FMI_2 */
	comp->handles=0;
#if defined(BUILD_LIBFMI)
	{
		extern void DymosimEnsureDLLLoaded(const char*const*toLoad, void***handles, int load, const char* fmuDLLpath, const char* const* usedLibraries);
		extern const char*dllLibraryPath[];
		extern const char* usedLibraries[];
		DYMOLA_STATIC const char* dymosimFMIPath();
		const char* fmiDLLPath = dymosimFMIPath();
		if(dllLibraryPath != 0 && dllLibraryPath[0] != 0){
			DymosimEnsureDLLLoaded(dllLibraryPath, &comp->handles, 1, fmiDLLPath, usedLibraries);
		}
	}
#elif defined(_MSC_VER) && !defined(NO_EXTERNAL_DLL)
	{
		/* For Loading DLLs from the model. This variant assumes absolute paths and non-wide file names*/
		extern const char*dllLibraryPath[];
		int numDlls,i,maxLen=0;
		if (dllLibraryPath!=0) {
			for(numDlls=0;dllLibraryPath[numDlls];++numDlls) {
				int stri=(int) strlen(dllLibraryPath[numDlls]);
				if (stri>maxLen) maxLen=stri;
			}
			if (numDlls>0) {
				char*s;
				DYMOLA_STATIC const char* dymosimFMIPath();
				int fmiPathLen = (int) strlen(dymosimFMIPath());
				if(fmiPathLen>maxLen) maxLen=fmiPathLen;
				comp->handles=(void**)ALLOCATE_MEMORY(numDlls, sizeof(void*));
				s=(FMIChar*)ALLOCATE_MEMORY(maxLen+1,sizeof(FMIChar));
				if (s==0 || comp->handles==0) goto fail;
				for(i=0;i<numDlls;++i) {
					const char*libI=dllLibraryPath[i];
					if (libI[0]=='!' && libI[1]=='!') {libI+=2;}
					else if (libI[0]=='!' && strlen(libI)>19 && libI[9]=='_' && libI[18]=='!' ) {libI+=19;}
					comp->handles[i]=0;
					strcpy(s,libI);
#if _WIN32_WINNT >= 0x0502
					if (s[0]=='.' && s[1]=='\\') {
						/* Special handling of local DLLs */
						strcpy(s, dymosimFMIPath());
						if (s[0]==0) strcpy(s,libI);
						else libI+=2;
					}
					/* This is the good way */
					PathRemoveFileSpecA(s);
					SetDllDirectoryA(s);
#endif
					comp->handles[i]=(void*)LoadLibraryA(libI);
#if _WIN32_WINNT >= 0x0502
					SetDllDirectory(0);
#endif
				}
				FREE_MEMORY(s);
			}
		}
	}
#endif
	/* Initialize to NULL to facilitate freeing om memory */
	comp->dstruct = NULL;
	comp->istruct = NULL;
	comp->states = comp->derivatives = comp->parameters = comp->inputs = comp->outputs = comp->auxiliary =
		comp->crossingFunctions = comp->statesNominal = comp->DAEDerivatives = NULL;
	comp->basetype = NULL;

	comp->isCoSim = FMIFalse; /*Default, change later if Co-Sim*/
	/* set sensible default start time */
	comp->time = 0;
	comp->logbufp = comp->logbuf;
#if (FMI_VERSION >= FMI_3)
	comp->nRequiredIntermediateInputs = 0;
	comp->nRequiredIntermediateOutputs = 0;
#endif
#if (FMI_VERSION >= FMI_3)
	comp->loggMessage = logMessage;
	comp->ie = instanceEnvironment;
#elif (FMI_VERSION >= FMI_2)
	comp->functions = funcs;
#else
	if(funcs && funcs->logger)
		comp->functions.logger = funcs->logger;
	comp->functions.allocateMemory = NULL;
	comp->functions.freeMemory = NULL;
#endif
	comp->instanceName = util_strdup(instanceName);
	if (comp->instanceName == NULL) {
		goto fail;
	}
	comp->loggingOn = loggingOn;

	/* verify GUID */
	if (strcmp(fmuGUID, GUIDString) != 0) {
		util_logger(comp, instanceName, FMIError, loggBadCall, "Invalid GUID: %s, expected %s", fmuGUID, GUIDString);
		goto fail;
	}

	comp->maxRunTimeReached = FMIFalse;
	comp->oneSimulationTotalTimerStart = 0.0;

#ifndef FMU_SOURCE_CODE_EXPORT
	/* Initialize - which will give a predictable error for missing license (otherwise it will be detected later). Not needed for source code export. */
	{
		extern int InitializeDymosimRunTime();
		if (!InitializeDymosimRunTime()) {
			util_logger(comp, instanceName,
				FMIFatal, loggUndef, "The license file was not found. Use the environment variable \"DYMOLA_RUNTIME_LICENSE\" to specify your Dymola license file.\n");
			goto fail;
		}
	}
	{
		const double max_run_time = GetAdditionalReals(4);
		if (max_run_time > 0.0) {
			extern double dymosimtime2_(int action, double* tbegin);
			dymosimtime2_(0, &comp->oneSimulationTotalTimerStart);
		}
	}
#endif

	comp->dstruct = (struct BasicDDymosimStruct*) ALLOCATE_MEMORY(1, sizeof(struct BasicDDymosimStruct));
	comp->istruct = (struct BasicIDymosimStruct*) ALLOCATE_MEMORY(1, sizeof(struct BasicIDymosimStruct));
	if (comp->dstruct == NULL || comp->istruct == NULL) {
		goto fail;
	}
	comp->duser = (double*) comp->dstruct;
	comp->iuser = (int*) comp->istruct;
	if (comp->did) {
		(( struct DYNInstanceDataMinimal*)comp->did)->basicD=comp->dstruct;
		(( struct DYNInstanceDataMinimal*)comp->did)->basicI=comp->istruct;
	}

	setBasicStruct((double*) comp->dstruct, (int*) comp->istruct);

	extern void InitializeDymosimStruct(struct BasicDDymosimStruct* basicD, struct BasicIDymosimStruct* basicI);
	InitializeDymosimStruct(comp->dstruct, comp->istruct);

	{
		int nx, nx2, nu, ny, nw, np, nsp, nrel2, nrel, ncons, dae, nd, nxp, nwp, nwc, nhelp2, naux;
		size_t i =0;
		GetDimensions5(&nx, &nx2, &nu, &ny, &nw, &np, &nsp, &nrel2, &nrel, &ncons, &dae, &nd, &nxp, &nwp, &nwc, &nhelp2, &naux);

		comp->nxOrig = nx;
		if (dae && !GetAdditionalFlags(40)) {
			if (nx2 == 0) {
				util_logger(comp, comp->instanceName, FMIError, loggQiErr, 
					"DAE mode is enabled but the model is an ODE. Please disable DAE mode.");
				goto fail;
			}
			if (!GetAdditionalFlags(27)) {
				util_logger(comp, comp->instanceName, FMIError, loggSundials,
					"DAE mode is not supported when using CVode. Please disable DAE mode or switch to IDA.");
				goto fail;
			}
			if (GetAdditionalFlags(31) || (QJacobianDefined_ & 24) == 24) {
				util_logger(comp, comp->instanceName, FMIError, loggSundials,
					"Analytic Jacobians are not supported when integrating using DAE solver.");
				goto fail;
			}
			nx = nx + nx2;
			comp->DAEMode = FMITrue;
			util_logger(comp, comp->instanceName, FMIOK, loggSundials,
				"The translated model is a DAE. Integrating using DAE solver.");
		} else
			comp->DAEMode = FMIFalse;

		/* DAE Interface for Model Exchange */
		if (GetAdditionalFlags(40)) { 
			if (nx2 == 0) {
				util_logger(comp, comp->instanceName, FMIError, loggQiErr,
					"DAE interface for Model Exchange is enabled but the model is an ODE. Please disable the DAE interface.");
				goto fail;
			}
			if (!GetAdditionalFlags(31)) {
				util_logger(comp, comp->instanceName, FMIError, loggQiErr,
					"Generation of analytic Jacobians needs to be enabled for the DAE interface for Model Exchange.");
				goto fail;
			}
			nx = nx + nx2; /* states and derivatives vector are appended with algebraic variables and residuals respectively */
			comp->DAEMode = FMITrue;
		}

		comp->nStates = nx;
		comp->nIn = nu;
		comp->nOut = ny;
		comp->nAux = nw;
		comp->nPar = np;
		comp->nSPar = nsp;
		comp->nCross = 2 * nrel;
		comp->nDStates = nd;
		comp->nPrevious = nxp;
		comp->nConstAux = nwc;

		/* Guard against zero value for size by adding one */
		comp->states = (FMIReal*) ALLOCATE_MEMORY(comp->nStates + 1, sizeof(FMIReal));
		comp->derivatives = (FMIReal*) ALLOCATE_MEMORY(comp->nStates + 1, sizeof(FMIReal));
		comp->parameters = (FMIReal*) ALLOCATE_MEMORY(comp->nPar + 1, sizeof(FMIReal));
		comp->inputs = (FMIReal*) ALLOCATE_MEMORY(comp->nIn + 1, sizeof(FMIReal));
		comp->outputs = (FMIReal*) ALLOCATE_MEMORY(comp->nOut + 1, sizeof(FMIReal));
		comp->auxiliary = (FMIReal*) ALLOCATE_MEMORY(comp->nAux + 1, sizeof(FMIReal));
		comp->crossingFunctions = (FMIReal*) ALLOCATE_MEMORY(comp->nCross + 1, sizeof(FMIReal));
		comp->statesNominal = (FMIReal*) ALLOCATE_MEMORY(comp->nStates + 1, sizeof(FMIReal));
		comp->sParameters = (FMIChar**) ALLOCATE_MEMORY(comp->nSPar + 1, sizeof(FMIChar*));
		comp->oldStates = (FMIReal*) ALLOCATE_MEMORY(comp->nStates + 1, sizeof(FMIReal));
		comp->dStates = (FMIReal*) ALLOCATE_MEMORY(comp->nDStates + 1, sizeof(FMIReal));
		comp->previousVars = (FMIReal*) ALLOCATE_MEMORY(comp->nPrevious + 1, sizeof(FMIReal));
		if (comp->DAEMode) { comp->DAEDerivatives = (FMIReal*) ALLOCATE_MEMORY(comp->nStates + 1, sizeof(FMIReal)); }

		GetNbrDynArray(&comp->dynArraySize.nbrReal, &comp->dynArraySize.nbrInt, &comp->dynArraySize.nbrStr);
		{
			size_t totSize = comp->dynArraySize.nbrReal + comp->dynArraySize.nbrInt + comp->dynArraySize.nbrStr;
			if (totSize) 
				comp->dynArraySize.dim = (double*)ALLOCATE_MEMORY(totSize, sizeof(double));
			else
				comp->dynArraySize.dim = NULL;

		}

	}
#if (FMI_VERSION >= FMI_2)
	{
		comp->ClockZeroVector = (FMIBoolean*) ALLOCATE_MEMORY(FMIClockValueReferences_[0]+1, sizeof(FMIBoolean));
	}
#endif

	if (comp->states == NULL || comp->derivatives == NULL || comp->parameters == NULL ||
		comp->inputs == NULL || comp->outputs == NULL || comp->auxiliary == NULL ||
		comp->crossingFunctions == NULL || comp->statesNominal == NULL || comp->sParameters == NULL ||
		(comp->DAEMode && comp->DAEDerivatives == NULL)) {
			goto fail;
	}
	/*Temporary fmiString pointers to retreve  original fmiStrings when calling reset
	 allocated if needed in reset*/
	comp->tsParameters = NULL; 
	/* default values */
	for (i = 0; i < comp->nStates; i++) {
		comp->statesNominal[i] = 1.0;
	}
	
	comp->mStatus = modelInstantiated;
	comp->storeResult = FMIFalse;
	comp->enforceRefresh = FMIFalse;
	comp->iData = NULL;

#if (FMI_VERSION >= FMI_2)
	jacData = &comp->jacData;
	jacData->jacA = jacData->jacB = jacData->jacC = jacData->jacD = NULL;
	jacData->jacV = jacData->jacVTmp1 = jacData->jacVTmp2 = jacData->jacZ = jacData->jacZTmp1 = jacData->jacZTmp2 = NULL;
	jacData->nJacA = jacData->nJacB = jacData->nJacC = jacData->nJacD = 0;
	jacData->nJacV = jacData->nJacZ = 0;
	jacData->stateStore = 0;
	comp->recalJacobian = 1;
#endif


	/* FMI API does not require caller to set start values, so must fetch start values for
	   states and parameters (other variables are initiated by dsblock_) */
	declareNew2_(comp->states, comp->derivatives, comp->parameters, comp->inputs, comp->outputs, comp->auxiliary, comp->statesNominal, (void**) comp->sParameters, &QiErr, 0, 0);
	for(i = 0; i < comp->nStates; ++i){
		if(comp->statesNominal[i] == 0.0)
			comp->statesNominal[i] = 1.0;
	}
	if (comp->DAEDerivatives) {
		for (i = 0; i < comp->nStates; ++i)
			comp->DAEDerivatives[i] = comp->derivatives[i];
	}
	for(i=0; i < comp->nSPar; ++i){
		FMIString s=(comp->sParameters)[i];
		size_t len;
		len=strlen(s);
		if (len>maxStringSize) len=maxStringSize;
		(comp->sParameters)[i] = (FMIChar*) ALLOCATE_MEMORY(maxStringSize+1, sizeof(FMIChar));
		memcpy((comp->sParameters)[i], s, len+1);
		(comp->sParameters)[i][maxStringSize]='\0';
	}

	if (QiErr != 0) {
		util_logger(comp, comp->instanceName, FMIError, loggQiErr,
			"%s: declare_ failed, QiErr = %d", label, QiErr);
		goto fail;
	}	
	/* Default values for setup experiment */
	comp->tStart = 0;
	comp->StopTimeDefined = FMIFalse;
	comp->tStop = 0;
	comp->relativeToleranceDefined = FMIFalse;
	comp->relativeTolerance = 0;

	comp->valWasSet = 0;
	comp->reinitCvode = 0;
	comp->eventIterRequired = 0;
#if FMI_VERSION == FMI_2
	comp->AdvancedDiscreteStates=fmiClockedStates;
#endif
	comp->hycosimInputEvent = 0;
	comp->nextResultSampleTime = 1e100;
	comp->fmi2ComputeInit = 0;

	if (!dyn_allowMultipleInstances)
		globalComponent = comp;

	comp->useInputSmoother = 2*GetAdditionalFlags(29);
	comp->usePredictorCompensation = GetAdditionalFlags(30);
	comp->useInputSmoother += GetAdditionalFlags(35);

#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
	/* for serialization using TPL */
	comp->tplDidBin = (tpl_bin*)ALLOCATE_MEMORY(1, sizeof(tpl_bin));
	if (comp->tplDidBin == NULL) {
		goto fail;
	}
#endif /* FMI_2 */

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s completed", label, QiErr);
	return comp;

fail:
	if (comp != NULL) {
		FMIString iName = instanceName != NULL ? instanceName : "";
		util_logger(comp, iName, FMIFatal, loggUndef, "Instantiation failed");
		fmiFreeModelInstance_(comp);
	}
	return NULL;
}

extern void delayBuffersClose(void);
extern void delayBuffersCloseNew(struct DYNInstanceData*);
extern void* dynExternalObjectFirst(struct DYNInstanceData* did);
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC void fmiFreeModelInstance_(FMIComponent c)
{
	Component* comp = (Component*)c;
#if (FMI_VERSION >= FMI_2)
	JacobianData* jacData;
#endif
	int i;
	if (comp == NULL) {
		return;
	}
	fmiUser_Free();
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "fmiFreeModelInstance");
	if (comp->did) {
		/* condition for external object destruction: external object either not reference counted or reference count reached 0 */
		/* if there are not external objects at all, the destruction will be skipped in dsblock5.c (since before) */
		delayBuffersCloseNew(comp->did);
	} 	else {
		delayBuffersClose();
	}
	assert(comp->instanceName != NULL);

	FREE_MEMORY(comp->instanceName);

	FREE_MEMORY(comp->dstruct);
	FREE_MEMORY(comp->istruct);

	FREE_MEMORY(comp->states);
	FREE_MEMORY(comp->derivatives);
	FREE_MEMORY(comp->parameters);
	FREE_MEMORY(comp->inputs);
	FREE_MEMORY(comp->outputs);
	FREE_MEMORY(comp->auxiliary);
	FREE_MEMORY(comp->crossingFunctions);
	for (i = (int)comp->nSPar - 1; i >= 0; --i) {
		FREE_MEMORY((comp->sParameters)[i]);
	}
	FREE_MEMORY(comp->sParameters);
	FREE_MEMORY(comp->statesNominal);
	FREE_MEMORY(comp->oldStates);
	FREE_MEMORY(comp->dStates);
	FREE_MEMORY(comp->previousVars);
	FREE_MEMORY(comp->ClockZeroVector);


	if (comp->tsParameters != NULL) {
		FREE_MEMORY(comp->tsParameters);
	}

	if (comp->DAEDerivatives != NULL) {
		FREE_MEMORY(comp->DAEDerivatives);
	}

#if (FMI_VERSION >= FMI_3)
	comp->nRequiredIntermediateInputs = 0;
	comp->nRequiredIntermediateOutputs = 0;
#endif
#if (FMI_VERSION >= FMI_2)
	jacData = &comp->jacData;
	FREE_MEMORY(jacData->jacA);
	FREE_MEMORY(jacData->jacB);
	FREE_MEMORY(jacData->jacC);
	FREE_MEMORY(jacData->jacD);
	FREE_MEMORY(jacData->jacV);
	FREE_MEMORY(jacData->jacVTmp1);
	FREE_MEMORY(jacData->jacVTmp2);
	FREE_MEMORY(jacData->jacZ);
	FREE_MEMORY(jacData->jacZTmp1);
	FREE_MEMORY(jacData->jacZTmp2);
	FREE_MEMORY(jacData->updatedJacobian);
	
#endif /* FMI_2 */

	if (comp->handles) {
#if defined(BUILD_LIBFMI)
		{
			extern void DymosimEnsureDLLLoaded(const char*const*toLoad, void***handles, int load, const char* fmuDLLpath, const char* const* usedLibraries);
			extern const char*dllLibraryPath[];
			extern const char* usedLibraries[];
			if (dllLibraryPath != 0 && dllLibraryPath[0] != 0) {
				DymosimEnsureDLLLoaded(dllLibraryPath, &comp->handles, 0, NULL, usedLibraries);
			}
		}
#elif defined(_MSC_VER) && !defined(NO_EXTERNAL_DLL)
		extern const char*dllLibraryPath[];
		int numDlls,i;
		if (dllLibraryPath!=0) {
			for(numDlls=0;dllLibraryPath[numDlls];++numDlls);
			for(i=0;i<numDlls;++i) {
				FreeLibrary(comp->handles[i]);
				comp->handles[i]=0;
			}
		}
		FREE_MEMORY(comp->handles);
#endif
	}
	if (comp->did) {
		if (dyn_allowMultipleInstances) {
			extern void EnsureMarkFree3();
			EnsureMarkFree3();
		}
		FREE_MEMORY(comp->did);
	}

#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
	/* for serialization using TPL */
	FREE_MEMORY(comp->tplDidBin);
#endif /* FMI_2 */
  
	FREE_MEMORY(comp->dynArraySize.dim);

	FREE_MEMORY(comp);
	if (!dyn_allowMultipleInstances) {
		assert(globalComponent == c || globalComponent == NULL);
		globalComponent = NULL;
	}

#ifndef FMU_SOURCE_CODE_EXPORT
	{
		extern void UnInitializeDymosimRunTime();
		UnInitializeDymosimRunTime();
	}
#endif
#ifdef MEMLEAK_DEBUG
	memdebug_check();
#endif
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetTime_(FMIComponent c, FMIReal time)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3SetTime";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2SetTime";
#else
	static FMIString label = "fmiSetTime";
#endif
	Component* comp = (Component*) c;
	/* avoid cluttering the code with check if time == comp->time to handle odd uses of this function
	   what complicates it is that if in modelEventModeExit, even same time should be considered */
#if (FMI_VERSION >= FMI_2)
	if (comp->mStatus == modelEventModeExit) {
		if (comp->nStates != 0) {
			util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: only allowed for discrete models when not in continuous time mode", label);
			return util_error(comp);
		}
		/* resuse this mode also to discrete models for convenience */
		comp->mStatus = modelContinousTimeMode;
	} else if (comp->mStatus != modelContinousTimeMode && comp->mStatus != modelInstantiated)  {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: not allowed in this state", label);
		return util_error(comp);
	}
#endif
	comp->time = time;
	comp->icall = iDemandStart;
	comp->recalJacobian = 1;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s to %g", label, time);
	return FMIOK;
}

#ifndef FMU_SKIP_MODEL_EXCHANGE
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetContinuousStates_(FMIComponent c, const FMIReal x[], size_t nx)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3SetContinuousStates";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2SetContinuousStates";
#else
	static FMIString label = "fmiSetContinuousStates";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;

	if (nx != comp->nStates) {
		if (!GetAdditionalFlags(40)) {
			status = FMIWarning;
			util_logger(comp, comp->instanceName, status, loggBadCall,
				"%s: argument nx = %u is incorrect, should be %u", label, nx, comp->nStates);
		}
		if (nx > comp->nStates) {
			/* truncate */
			nx = comp->nStates;
		}
	}
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	memcpy(comp->states, x, nx * sizeof(FMIReal));
	/* reset caching */
	comp->valWasSet = 1;
	comp->icall = iDemandStart;
	comp->recalJacobian = 1;
	return status;
}

#endif /* FMU_SKIP_MODEL_EXCHANGE */

#if (FMI_VERSION >= FMI_2)
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiEnterModelInitializationMode_(FMIComponent c, FMIBoolean toleranceControlled, FMIReal relativeTolerance)
{
	static FMIString label = "fmi2EnterInitializationMode";
	Component* comp = (Component*) c;
	FMIStatus status;

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);

	status = util_initialize_model(c, toleranceControlled, relativeTolerance, FMIFalse);
	if (status != FMIOK) {
		return status;
	}
	comp->mStatus = modelInitializationMode;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s completed", label);
	return status;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiExitModelInitializationMode_(FMIComponent c)
{
	static FMIString label = "fmi2ExitInitializationMode";
	Component* comp = (Component*) c;
	FMIStatus status;

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);
	status = util_exit_model_initialization_mode(c, label, modelEventMode);
	comp->firstEventCall=FMITrue;
	if (status != FMIOK) {
		return status;
	}
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s completed", label);
	return FMIOK;
}

#else /* FMI_2 */

/* ---------------------------------------------------------------------- */

DYMOLA_STATIC FMIStatus fmiInitialize_ (FMIComponent c, FMIBoolean toleranceControlled, FMIReal relativeTolerance, FMIEventInfo* eventInfo)
{
	static FMIString label = "fmiInitialize";
	Component* comp = (Component*) c;
	FMIStatus status;
	if(fmiUser_Initialize())
		return util_error(comp);
	/* for co-simulation, this initialization is only a subset */
	if (!comp->isCoSim) {
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);
	}
	status = util_initialize_model(c, toleranceControlled, relativeTolerance, FMITrue);
	if (status != fmiOK ) {
		return status;
	}

	eventInfo->terminateSimulation = FMIFalse;
	eventInfo->upcomingTimeEvent = (comp->dstruct->mNextTimeEvent < TIME_INFINITY) ? FMITrue : FMIFalse;
	if (eventInfo->upcomingTimeEvent == FMITrue) {
		eventInfo->nextEventTime = comp->dstruct->mNextTimeEvent;
	}

	/* for co-simulation, this initialization is only a subset */
	if (!comp->isCoSim) {
		comp->mStatus = modelContinousTimeMode;
		util_logger(comp, comp->instanceName, status, loggFuncCall, "%s completed", label);
	}

	comp->eventIterationOnGoing = 0;
	return FMIOK;
}
#endif /* FMI_2 */

#ifndef FMU_SKIP_MODEL_EXCHANGE

#if (FMI_VERSION >= FMI_2)
/* ---------------------------------------------------------------------- */

DYMOLA_STATIC FMIStatus fmiEnterEventMode_(FMIComponent instance){
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3EnterEventMode";
#else
	static FMIString label = "fmi2EnterEventMode";
#endif
	Component* comp = (Component*) instance;
	
	if (comp->mStatus != modelContinousTimeMode) {
		util_logger(comp, comp->instanceName, FMIError, "", "%s: may only be called in continuous time mode", label);
		return util_error(comp);
	}
	comp->mStatus = modelEventMode;
	comp->firstEventCall = FMITrue;
	return FMIOK;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiEnterContinuousTimeMode_(FMIComponent c)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3EnterContinuousTimeMode";
#else
	static FMIString label = "fmi2EnterContinuousTimeMode";
#endif
	Component* comp = (Component*) c;
	
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);
	if (comp->mStatus != modelEventModeExit) {
		util_logger(comp, comp->instanceName, FMIError, "", "%s: may only be called when exited event mode", label);
		return util_error(comp);
	}
	ME_RESULT_SAMPLE(FMITrue);
	comp->mStatus = modelContinousTimeMode;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s done", label);
	return FMIOK;
}

/* ---------------------------------------------------------------------- */
#if (FMI_VERSION >= FMI_3)
DYMOLA_STATIC FMIStatus fmi3UpdateDiscreteStates_(FMIComponent c, fmi3Boolean* discreteStatesNeedUpdate,  fmi3Boolean* terminateSimulation,
												  fmi3Boolean* nominalsOfContinuousStatesChanged, fmi3Boolean* valuesOfContinuousStatesChanged,
												  fmi3Boolean* nextEventTimeDefined, fmi3Float64* nextEventTime)
{
	FMIEventInfo ev = {0};
	FMIEventInfo* eventInfo = &ev;
	static FMIString label = "fmi3UpdateDiscreteStates";

#else
DYMOLA_STATIC FMIStatus fmiNewDiscreteStates_(FMIComponent c, FMIEventInfo* eventInfo)
{	
	static FMIString label = "fmi2NewDiscreteStates";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	int QiErr = 0;
	FMIBoolean converged = 0;
	size_t i = 0;

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);

	/* for co-simulation, sampling is handled at higher level */
	if (comp->isCoSim == FMIFalse) {
		ME_RESULT_SAMPLE(FMITrue);
	}
	memcpy(comp->oldStates,comp->states, comp->nStates*sizeof(FMIReal));
	if( (comp->nStates == 0 || comp->isCoSim) && comp->mStatus == modelContinousTimeMode || 
		comp->mStatus == modelCsPreEventMode ){
		comp->mStatus = modelEventMode;
	}
	/* configure actual event iteration */
	switch (comp->mStatus) {
		case modelEventModeExit:
			/* allowed to restart the event iteration again */
			comp->mStatus = modelEventMode;
			/* fall-through */
		case modelEventMode:
		case modelEventMode2:
			if(comp->valWasSet && !comp->firstEventCall){
				SetDymolaOneIteration(comp->did, 5);
				QiErr = util_refresh_cache(comp, iDemandEventHandling, NULL, &converged);
				if (QiErr != 0) {
					goto eventDone;
				}
			}
			SetDymolaOneIteration(comp->did, 3);
			break;
		default:
			util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: may only be called in event mode", label);
			return util_error(comp);
	}
	comp->icall = 0;
	QiErr = util_refresh_cache(comp, iDemandEventHandling, NULL, &converged);
eventDone:
	eventInfo->valuesOfContinuousStatesChanged = FMIFalse;
	for (i=0; i<comp->nStates; ++i){
		if(comp->states[i] != comp->oldStates[i]){
			eventInfo->valuesOfContinuousStatesChanged = FMITrue;
		}
	}
	if (QiErr != 0) {
		if (QiErr == -999) {
			util_logger(comp, comp->instanceName, FMIOK, loggUndef, "%s: simulation terminated by model", label);
			eventInfo->terminateSimulation = FMITrue;
			eventInfo->newDiscreteStatesNeeded = FMIFalse;
			eventInfo->nominalsOfContinuousStatesChanged= FMIFalse;
			/* eventInfo->valuesOfContinuousStatesChanged = (comp->nStates > 0) ? FMITrue : FMIFalse; */
			eventInfo->nextEventTimeDefined = FMIFalse;
			eventInfo->nextEventTime = 1.0e37;
			comp->mStatus = modelEventModeExit;
			comp->terminationByModel = FMITrue;
#if (FMI_VERSION >= FMI_3)
			goto setEV;
#endif
			return FMIOK;
		} else {
			util_logger(comp, comp->instanceName, FMIError, loggQiErr,
				"%s: dsblock_ failed, QiErr = %d", label, QiErr);
			return util_error(comp);
		}
	}
	{
#if FMI_VERSION <  FMI_3
		if(FMIClockValueReferences_[0]){
			/*Turn of all manual clocks*/
			fmiSetBoolean_(c, &FMIClockValueReferences_[1], FMIClockValueReferences_[0], comp->ClockZeroVector);
		}
#endif
	}
	
	if (converged == FMIFalse)
	{
		eventInfo->newDiscreteStatesNeeded = FMITrue;
		comp->mStatus = modelEventMode2;
		comp->eventIterRequired = 1;
	} else {
		eventInfo->newDiscreteStatesNeeded = FMIFalse;
		comp->mStatus = modelEventModeExit;
		comp->eventIterRequired = 0;
	}

	eventInfo->terminateSimulation = FMIFalse;
	eventInfo->nominalsOfContinuousStatesChanged= FMIFalse;
	eventInfo->nextEventTimeDefined = (comp->dstruct->mNextTimeEvent < (1.0E37 - 1)) ? FMITrue : FMIFalse;
	if (eventInfo->nextEventTimeDefined == FMITrue) {
		eventInfo->nextEventTime = comp->dstruct->mNextTimeEvent;
	}
	comp->hycosimInputEvent = 0;
	comp->recalJacobian = 1;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s completed", label);
#if (FMI_VERSION >= FMI_3)
setEV:
	*discreteStatesNeedUpdate = eventInfo->newDiscreteStatesNeeded;
	*terminateSimulation = eventInfo->terminateSimulation;
	*nominalsOfContinuousStatesChanged = eventInfo->nominalsOfContinuousStatesChanged;
	*valuesOfContinuousStatesChanged = eventInfo->valuesOfContinuousStatesChanged;
	*nextEventTimeDefined = eventInfo->nextEventTimeDefined;
	*nextEventTime = eventInfo->nextEventTime;
#endif
	return FMIOK;
}
#else
DYMOLA_STATIC FMIStatus fmiEventUpdate_(FMIComponent c, FMIBoolean intermediateResults, FMIEventInfo* eventInfo)
{
	static FMIString label = "fmiEventUpdate";
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	int QiErr = 0;
	FMIBoolean converged = 0;

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s...", label);

	if (comp->mStatus != modelContinousTimeMode) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
			"%s: initialization must be done before event updating is allowed", label);
		return util_error(comp);
	}

	ME_RESULT_SAMPLE(FMITrue);
	status = util_event_update(c, intermediateResults, eventInfo);
	if (status == FMIError) {
		return util_error(comp);
	} else if (status == FMIFatal) {
		return FMIFatal;
	}

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s completed", label);
	return status;
}

#endif /* FMI_2 */

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiCompletedIntegratorStep_(FMIComponent c,
#if (FMI_VERSION >= FMI_2)
	FMIBoolean noSetFMUStatePriorToCurrentPoint,
	FMIBoolean* enterEventMode,
	FMIBoolean* terminateSimulation)
#else
	FMIBoolean* callEventUpdate)
#endif
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3CompletedIntegratorStep";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2CompletedIntegratorStep";
#else
	static FMIString label = "fmiCompletedIntegratorStep";
#endif
	Component* comp = (Component*) c;
	int QiErr = 0;

	if (comp->icall < iDemandDerivative) {
		QiErr = util_refresh_cache(comp, iDemandDerivative, label, NULL);
		if (QiErr != 0) {
			if (QiErr == -999) {
				util_logger(comp, comp->instanceName, FMIOK, loggUndef, "%s: simulation terminated by model", label);
				comp->terminationByModel = FMITrue;
			}else{
				return util_error(comp);
			}
		}
	}

	ME_RESULT_SAMPLE(FMIFalse);

#if (FMI_VERSION >= FMI_2)
	if (comp->storeResult == FMITrue) {
		if (noSetFMUStatePriorToCurrentPoint) {
			if(result_flush(comp)){
				util_logger(comp, comp->instanceName, FMIFatal, loggUndef,
					"%s: Error when sampling result file, out of memory\n", label);
				return FMIFatal;
			}
		}
	}
	*terminateSimulation = comp->terminationByModel;
	/* only applies to step event */
	*enterEventMode = comp->istruct->mTriggerStepEvent ? FMITrue : FMIFalse;
#else
	*callEventUpdate = comp->istruct->mTriggerStepEvent ? FMITrue : FMIFalse;
#endif
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return FMIOK;
}

#endif /* FMU_SKIP_MODEL_EXCHANGE */

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC void copyDStatesToDid(struct DYNInstanceData*did_, double* dStates, double* previousVars);
DYMOLA_STATIC void copyDStatesFromDid(struct DYNInstanceData*did_, double* dStates, double* previousVars);
DYMOLA_STATIC int dsblock_did(int *idemand_, int *icall_,
	double *time, double x_[], double xd_[], double u_[],
	double dp_[], int ip_[], Dymola_bool lp_[],
	double f_[], double y_[], double w_[], double qz_[],
	double duser_[], int iuser_[], void*cuser_[], struct DYNInstanceData*,
	int *ierr_);

#if (FMI_VERSION >= FMI_2)
DYMOLA_STATIC FMIStatus fmiTerminateModel_(FMIComponent c)

#else
DYMOLA_STATIC FMIStatus fmiTerminate_(FMIComponent c)
#endif
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3Terminate";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2Terminate";
#else
	static FMIString label = "fmiTerminate";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	if (comp->mStatus == modelInstantiated) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: FMU not initialized", label);
		return util_error(comp);
	}
	if (comp->mStatus == modelTerminated) {
		util_logger(comp, comp->instanceName, FMIWarning, loggBadCall, "%s: already terminated, ignoring call", label);
		return FMIWarning;
	}
	if(fmiUser_Terminate())
		return util_error(comp);
	if (!comp->isCoSim) {
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s",label);
		if(comp->QiErr == 0 && comp->terminationByModel == FMIFalse){
			/*Special case for terminal, call dsblock_ directly instead of
			using util_refresh_cache to avoid messy logic*/
			int terminal = iDemandTerminal;
			copyDStatesToDid(comp->did,comp->dStates,comp->previousVars);
			if (comp->did) {
				globalComponent2=comp;
				dsblock_did(&terminal, &comp->icall, &comp->time, comp->states, comp->DAEMode && comp->DAEDerivatives ? comp->DAEDerivatives : 0,
					comp->inputs, comp->parameters, 0, 0, comp->derivatives, 
					comp->outputs, comp->auxiliary,                                
					comp->crossingFunctions, comp->duser, comp->iuser,
					(void**) comp->sParameters, comp->did, &comp->QiErr);
				globalComponent2=0;
			} else {
				dsblock_(&terminal, &comp->icall, &comp->time, comp->states, comp->DAEMode && comp->DAEDerivatives ? comp->DAEDerivatives : 0,             
					comp->inputs, comp->parameters, 0, 0, comp->derivatives,       
					comp->outputs, comp->auxiliary,                                
					comp->crossingFunctions, comp->duser, comp->iuser,
					(void**) comp->sParameters, &comp->QiErr);
			}
			copyDStatesFromDid(comp->did,comp->dStates,comp->previousVars);
			if (comp->QiErr>=-995 && comp->QiErr<=-990) comp->QiErr=0; /* Ignore special cases for now */
			if(!(comp->QiErr == 0 || comp->QiErr==-999)){
				status = FMIError;
				util_logger(comp, comp->instanceName, FMIError, loggQiErr,
					"%s: calling terminal section of dsblock_ failed, QiErr = %d",
					label,comp->QiErr);
			}
		}
	}
	util_print_dymola_timers(c);
#ifndef FMU_SOURCE_CODE_EXPORT
	if (comp->storeResult == FMITrue) {
		if(result_teardown(comp)){
			status = FMIFatal;
			util_logger(comp, comp->instanceName, status, loggUndef,
				"%s: Error when terminating result file, out of memory\n", label);
			return status;
		}
	}
#endif /* FMU_SOURCE_CODE_EXPORT */

	comp->mStatus = modelTerminated;
	return status;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetDerivatives_(FMIComponent c, FMIReal derivatives[], size_t nx)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3GetDerivatives";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2GetDerivatives";
#else
	static FMIString label = "fmiGetDerivatives";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	int QiErr = 0;
	if(	comp->mStatus == modelInstantiated){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
#if (FMI_VERSION >= FMI_2)
			"%s: fmi2EnterInitializationMode must be called before calling %s", label, label);
#else
			"%s: fmiInitializeModel must be called before calling %s", label, label);
#endif
		return util_error(comp);
	}
	if (nx != comp->nStates) {
		if (!GetAdditionalFlags(40)) {
			status = FMIWarning;
			util_logger(comp, comp->instanceName, status, loggBadCall,
				"%s: argument nx = %u is incorrect, should be %u", label, nx, comp->nStates);
		}
		if (nx > comp->nStates) {
			/* truncate */
			nx = comp->nStates;
		}
	}

	if (comp->icall < iDemandDerivative || comp->fmi2ComputeInit) {
		/* refresh cache */
		int QiErr = util_refresh_cache(comp, iDemandDerivative, label, NULL);
		if (QiErr != 0) {
			comp->icall = 0;
			comp->valWasSet = 1;
			return (QiErr == 1) ? FMIDiscard : util_error(comp); 
		}
	}

	memcpy(derivatives, comp->derivatives, nx * sizeof(FMIReal));
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return status;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetEventIndicators_(FMIComponent c, FMIReal eventIndicators[], size_t ni)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3GetEventIndicators";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2GetEventIndicators";
#else
	static FMIString label = "fmiGetEventIndicators";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	if(	comp->mStatus == modelInstantiated){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
#if (FMI_VERSION >= FMI_3)
			"%s: fmi3EnterInitializationMode must be called before calling %s", label, label);
#elif (FMI_VERSION >= FMI_2)
			"%s: fmi2EnterInitializationMode must be called before calling %s", label, label);
#else
			"%s: fmiInitializeModel must be called before calling %s", label, label);
#endif
		return util_error(comp);
	}
	if (ni != comp->nCross) {
		status = FMIWarning;
		util_logger(comp, comp->instanceName, status, loggBadCall,
			"%s: argument ni = %u is incorrect, should be %u", label, ni, comp->nCross);
		if (ni > comp->nCross) {
			/* truncate */
			ni = comp->nCross;
		}
	}

	if (comp->icall < iDemandCrossingFunction && comp->terminationByModel == FMIFalse || comp->fmi2ComputeInit) {
		/* refresh cache */
		int QiErr = util_refresh_cache(comp, iDemandCrossingFunction, label, NULL);
		if (QiErr != 0) {
			comp->icall = 0;
			comp->valWasSet = 1;
			return (QiErr == 1) ? FMIDiscard : util_error(comp); 
		}
	}

	memcpy(eventIndicators, comp->crossingFunctions, ni * sizeof(FMIReal));
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return status;
}

#ifndef FMU_SKIP_MODEL_EXCHANGE

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetContinuousStates_(FMIComponent c, FMIReal states[], size_t nx)
{
#if (FMI_VERSION >= FMI_3)
	static FMIString label = "fmi3GetContinuousStates";
#elif (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2GetContinuousStates";
#else
	static FMIString label = "fmiGetContinuousStates";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
#if (FMI_VERSION >= FMI_2)
	if(	comp->mStatus == modelInstantiated){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
			"%s: fmiEnterInitializationMode must be called before calling %s", label, label);
		return util_error(comp);
	}
#endif
	if (nx != comp->nStates) {
		if (!GetAdditionalFlags(40)) {
			status = FMIWarning;
			util_logger(comp, comp->instanceName, status, loggBadCall,
				"%s: argument nx = %u is incorrect, should be %u", label, nx, comp->nStates);
		}
		if (nx > comp->nStates) {
			/* truncate */
			nx = comp->nStates;
		}
	}

	memcpy(states, comp->states, nx * sizeof(FMIReal));
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return status;
}

/* ---------------------------------------------------------------------- */
#if (FMI_VERSION >= FMI_3)
DYMOLA_STATIC FMIStatus fmi3GetNominalsOfContinuousStates_(FMIComponent c, FMIReal x_nominal[], size_t nx){
	static FMIString label = "fmi3GetNominalContinuousStates";
#elif (FMI_VERSION >= FMI_2)
DYMOLA_STATIC FMIStatus fmiGetNominalsOfContinuousStates_(FMIComponent c, FMIReal x_nominal[], size_t nx) {
	static FMIString label = "fmi2GetNominalsOfContinuousStates";
#else
DYMOLA_STATIC FMIStatus fmiGetNominalContinuousStates_(FMIComponent c, FMIReal x_nominal[], size_t nx) {
	static FMIString label = "fmiGetNominalContinuousStates";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	size_t i;

	if (nx != comp->nStates) {
		if (!GetAdditionalFlags(40)) {
			status = FMIWarning;
			util_logger(comp, comp->instanceName, status, loggBadCall,
				"%s: argument nx = %u is incorrect, should be %u", label, nx, comp->nStates);
		}
		if (nx > comp->nStates) {
			/* truncate */
			nx = comp->nStates;
		}
	}

	for (i = 0; i < nx; ++i) {
		if (comp->statesNominal[i] == 0.0)
			x_nominal[i] = 1.0;
		else
			x_nominal[i] = Dymola_abs(comp->statesNominal[i]);
	}

	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return status;
}

DYMOLA_STATIC FMIStatus fmiSetClock_(FMIComponent c, const FMIInteger clockIndex[], size_t nClockIndex, const FMIBoolean tick[], const FMIBoolean subactive[])
{
	static FMIString label = "fmi2SetClock";
	Component* comp = (Component*) c;
	size_t i = 0;
	FMIStatus status = FMIOK, status2 = FMIOK;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s Started...", label);
	for( i = 0; i < nClockIndex; ++i){
		if(clockIndex[i] > (int) FMIClockValueReferences_[0] || clockIndex[i] < 1){
			util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s FMU does not contain any clock with index %d, skiping clock", label, clockIndex[i]);
			return util_error(comp);
		}else{
#if FMI_VERSION < FMI_3
			status = fmiSetBoolean_(c, &FMIClockValueReferences_[clockIndex[i]], 1, &tick[i]);

			if(status != FMIOK){
				util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s Error when setting clock &d", label, clockIndex[i]);
				return util_error(comp);
			}
			if(subactive[i]){
				status = fmiSetBoolean_(c, &FMIClockFirstValueReferences_[clockIndex[i]], 1, &subactive[i]);
				if(status != FMIOK){
					util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s Error when setting clock &d", label, clockIndex[i]);
					return util_error(comp);
				}
			}
#endif
		}

	}
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s Compleated", label);
	return status;
}

DYMOLA_STATIC FMIStatus fmiGetClock_(FMIComponent c, const FMIInteger clockIndex[], size_t nClockIndex, FMIBoolean tick[])
{
	static FMIString label = "fmi2GetClock";
	Component* comp = (Component*) c;
	size_t i = 0;
	FMIStatus status = FMIOK, status2 = FMIOK;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s Started...", label);
	for( i = 0; i < nClockIndex; ++i){
		if(clockIndex[i] > (int) FMIClockValueReferences_[0] || clockIndex[i] < 1){
			util_logger(comp, comp->instanceName, FMIWarning, loggBadCall, "%s FMU does not contain any clock with index %d, skiping clock", label, clockIndex[i]);
		}else{
#if FMI_VERSION < FMI_3
			status = fmiGetBoolean_(c, &FMIClockValueReferences_[clockIndex[i]], 1, &tick[i]);
			if(status != FMIOK){
				util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "%s Error when retreving clock &d", label, clockIndex[i]);
				status2 = max(status,status2);
			}
#endif
		}
	}
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s Compleated", label);
	return status;
}

DYMOLA_STATIC FMIStatus fmiSetInterval_(FMIComponent c, const FMIInteger clockIndex[], size_t nClockIndex, FMIReal interval[])
{
	static FMIString label = "fmi2SetInterval";
	Component* comp = (Component*) c;
	FMIStatus status = FMIWarning;
	util_logger(comp, comp->instanceName, FMIWarning, loggBadCall, "%s is currently not supported call is ignored", label);
	return FMIWarning;
}

DYMOLA_STATIC FMIStatus fmiGetInterval_(FMIComponent c, const FMIInteger clockIndex[], size_t nClockIndex, FMIReal interval[])
{
	static FMIString label = "fmi2GetInterval";
	Component* comp = (Component*) c;
	FMIStatus status = FMIWarning;
	util_logger(comp, comp->instanceName, FMIWarning, loggBadCall, "%s is currently not supported call is ignored", label);
	return FMIWarning;
}

#if (FMI_VERSION < FMI_2)
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetStateValueReferences_(FMIComponent c, FMIValueReference vrx[], size_t nx)
{
	static FMIString label = "fmiGetStateValueReferences";
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
	size_t i;

	if (nx != comp->nStates) {
		if (!GetAdditionalFlags(40)) {
			status = FMIWarning;
			util_logger(comp, comp->instanceName, status, loggBadCall,
				"%s: argument nx = %u is incorrect, should be %u", label, nx, comp->nStates);
		}
		if (nx > comp->nStates) {
			/* truncate */
			nx = comp->nStates;
		}
	}

	for (i = 0; i < nx; i++) {
#ifndef FMU_SOURCE_SINGLE_UNIT
		extern unsigned int FMIStateValueReferences_[];
#endif
		vrx[i] = FMIStateValueReferences_[i];
	}
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return status;
}

#endif /* ndef FMI_2 */
#if FMI_VERSION >= FMI_3
DYMOLA_STATIC FMIStatus fmi3GetNumberOfEventIndicators_(fmi3Instance instance, size_t* nEventIndicators) {
    Component * comp = (Component*) instance;
	*nEventIndicators = comp->nCross;
	return FMIOK;
}

fmi3Status fmi3GetNumberOfContinuousStates_(fmi3Instance instance, size_t* nContinuousStates) {
    Component * comp = (Component*) instance;
	*nContinuousStates = comp->nStates;
	return FMIOK;
}
#endif

#endif /* FMU_SKIP_MODEL_EXCHANGE */
