/* The generic implementation of the FMI interface common for both ME and Co-simulation. */

/* need to include first so that correct files are included */
#include "conf.h"
#include "util.h"
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
#include "integration.h"
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */
#include "fmiFunctions_fwd.h"
#if (FMI_VERSION >= FMI_2)
/* for serialization of FMU state */
#if !(defined(NO_FILE) || defined(RTTWINCAT))
#include "cexch.h"
#include "tpl.h"
#endif
#endif /* FMI_2 */

#include <assert.h>
#include <math.h>
#include "adymosim.h"

/* ----------------- macros ----------------- */

#define ALLOC_AND_CHECK(dest, n, size)    	     					\
	dest = ALLOCATE_MEMORY(n, size);	                        	\
	if (dest == NULL) {												\
		goto mem_fail;                                              \
	} else {														\
		freeList[freeIndex++] = dest;								\
	}

#define ALLOC_AND_CHECK2(dest, n, type)    	     					\
	dest = (type*) ALLOCATE_MEMORY(n, sizeof(type));	            \
	if (dest == NULL) {												\
		goto mem_fail;                                              \
	} else {														\
		freeList[freeIndex++] = dest;								\
	}

/* ----------------- local function declarations ----------------- */


/* allocate memory for the FMU state */


#if (FMI_VERSION >= FMI_2)
static FMIStatus allocateFMUState(FMIFMUstate* FMUState, Component* source);

#if !(defined(NO_FILE) || defined(RTTWINCAT))
/* create TPL map */
static tpl_node* createTplMap(Component* comp, tpl_bin *tplDidBin);

/* local functions for TPL to use */
static void *tplMalloc(size_t size);
static void tplFree(void* ptr);
#endif
#endif /* FMI_2 */

/* find the largest iDemand value in a given array of value references */
static int getLargestIdemand(const FMIValueReference vr[], size_t nvr);
DYMOLA_STATIC void DYNPropagateDidToThread(struct DYNInstanceData* did);

#if (FMI_VERSION >= FMI_2)
static int copyVariables(Component* source, Component* target);
#endif

#if (FMI_VERSION >= FMI_3)
/*Check if vr is array and returns a new VR if thats the case*/
static DYM_INLINE FMIValueReference check_arrayVR(FMIValueReference vr, FMIBoolean* isArray);
#endif

static DYM_INLINE double* getCatPointer(Component* comp, FMICategory cat);

static DYM_INLINE FMIBoolean setIsAllowed(Component* comp, FMICategory cat, int index);

/* ----------------- local variables ----------------- */

/* when compiling as a single complilation unit, this is already defined */
#ifndef FMU_SOURCE_SINGLE_UNIT
/* only supports single instances */
Component* globalComponent = NULL;
Component* globalComponent2 = NULL;
#endif

/* Global logger function used by DymosimMessage routines */
DYMOLA_STATIC void fmu_logger(const char* message, int newline, int severity) 
{

	char hashHandeldMessage[4096];
	FMIStatus stat=FMIError;
	int i = 0;
	int j = 0;
	Component* comp;

	switch (severity){
		case 0: stat = FMIOK; break; 
        case 1: stat = FMIWarning; break;
        case 2: stat = FMIError;   break;
	}
		
	for(; j<sizeof(hashHandeldMessage)-1;++i,++j){
		if(message[i]=='\0'){
			hashHandeldMessage[j]='\0';
			break;
		} else if(message[i]=='#'){
			hashHandeldMessage[j++]='#';
		} else if (message[i] == '\n') {
			newline = 1;
			/* If there is a newline we should flush the buffer */
		}

		hashHandeldMessage[j]=message[i];
	}
	hashHandeldMessage[sizeof(hashHandeldMessage)-1]='\0';
		
	comp = globalComponent;
	if (comp == NULL) comp=globalComponent2;
	if (comp != NULL) {
		int bufcap = (int) (sizeof(comp->logbuf) - (comp->logbufp - comp->logbuf) - 1);
		if (newline || j >= bufcap) {
			util_logger(comp, comp->instanceName, stat, loggUndef, "%s", hashHandeldMessage);
		} else {
			util_buflogger(comp, comp->instanceName, stat, loggUndef, "%s", hashHandeldMessage);
		}
	}
}

/* ----------------- external variables ----------------- */

#ifndef DYMOLA_STATIC_IMPORT
#define DYMOLA_STATIC_IMPORT DYMOLA_STATIC
#endif

extern void delayBuffersCloseNew(struct DYNInstanceData*);
extern void delayBuffersClearExternalTables(struct DYNInstanceData*);
extern void* dynExternalObjectFirst(struct DYNInstanceData* did);
extern size_t getNbrExternalObjects();
extern size_t externalTableSize();
extern struct ExternalTable_* DymosimGetExternalObject2(struct DYNInstanceData*did_);
DYMOLA_STATIC size_t DYNGetMaxAuxStrLen();
DYMOLA_STATIC void DYNStrClear(struct DYNInstanceData* did);
DYMOLA_STATIC void DYNPropagateDidToThread(struct DYNInstanceData* did);
/* ----------------- unexposed function definitions ----------------- */


/* Enable storing of result from within FMU */
DYMOLA_STATIC void __enableResultStoring(FMIComponent c) {
	Component* comp = (Component*) c;
	comp->storeResult = FMITrue;
}

/* ----------------- function definitions ----------------- */

/* For 1.0 it belongs to Co-sim interface only */
#if (FMI_VERSION == FMI_2)
DYMOLA_STATIC const char* fmiGetTypesPlatform_()
{
	return fmi2TypesPlatform;
}
#endif

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC const char* fmiGetVersion_()
{
#if (FMI_VERSION >= FMI_3)
	return fmi3Version;
#elif (FMI_VERSION >= FMI_2)
	return fmi2Version;
#else
	return fmiVersion;
#endif
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetDebugLogging_(FMIComponent c, FMIBoolean loggingOn
#if (FMI_VERSION >= FMI_2)
, size_t nCategories, const FMIString categories[]
#endif
)
{
	static const char* logFunctionCall = "FunctionCalls";
	Component* comp = (Component*) c;
	size_t i = 0;
	comp->loggingOn = loggingOn;
#if (FMI_VERSION >= FMI_2)
	for(i = 0; i < nCategories; ++i){
		if(!strcmp(categories[i],logFunctionCall)){
			comp->loggFuncCall = FMITrue;
		}
	}
#endif
	return FMIOK;
}
#if (FMI_VERSION >= FMI_2)
static FMIString initFuncName = "fmi2EnterInitializationMode";
#else
static FMIString initFuncName = "fmiInitializeModel";
#endif




#if (FMI_VERSION >= FMI_3)

static const char* sFloat64 = "Float64";
static const char* sInt32 = "Int32";
static const char* sBoolean = "Boolean";
static const char* sInt64 = "Int64";
static const char* sUInt64 = "UInt64";


DYMOLA_STATIC FMIStatus FMIUpdateBeforeGet(FMIComponent c, const FMIString label, const FMIValueReference valueReferences[], size_t nVr, size_t nVal){
	Component* comp = (Component*) c;                                             
	int idemand = 0;
	static const char* get= "fmi3Get";
	if(comp->mStatus == modelInstantiated ){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
		"fmiGet%s: Not allowed before call of %s",label, initFuncName);
		return util_error(comp);
	}

	// For FMI3, array support makes size checking complicated
	if (nVr > nVal) {
		ERROR_RETURN_CHECK(checkSizes(comp, valueReferences, nVr, nVal, label, get));
	}

	idemand = getLargestIdemand(valueReferences, nVr);

    if (comp->mStatus != modelTerminated && ( comp->icall < idemand && comp->mStatus != modelInitializationMode || 
	    comp->mStatus == modelInitializationMode && comp->valWasSet || comp->enforceRefresh || comp->fmi2ComputeInit )) {
		int QiErr = 0;
		if(comp->enforceRefresh && idemand <1) idemand = 3;
        QiErr = util_refresh_cache(comp, idemand, NULL, NULL);
		if (QiErr != 0) {  
		    FMIStatus status = (QiErr == 1 && strcmp(label, "Float64") == 0) ? FMIDiscard : FMIError;
			util_logger(comp, comp->instanceName, status, loggQiErr, "fmiGet%s: dsblock_ failed, QiErr = %d",label, QiErr); 
			comp->icall = 0;
			comp->valWasSet = 1; 
			HANDLE_STATUS_RETURN(status);
		} 
	}
	return FMIOK;
}
extern void GetRealDymArrayValues(struct DYNInstanceData* did_, int index, double* data, int nbrData);
extern void GetIntegerDymArrayValues(struct DYNInstanceData* did_, int index, int* data, int nbrData);
//extern void GetStringDymArrayValues(struct DYNInstanceData* did_, int index, const char** data, int nbrData);
DYMOLA_STATIC FMIStatus fmi3GetDymArray(FMIComponent c, typePointer* tp, fmi3Types type, size_t ptIndx, size_t index, size_t nvals) {
	Component* comp = (Component*)c;

	switch (type)
	{
	case fmiTypeFloat64:
		GetRealDymArrayValues(comp->did, (int)index, &tp->rp[ptIndx], (int)nvals);
		break;
	case fmiTypeInt32:
	{
		size_t intIndex = index  - comp->dynArraySize.nbrReal;
		GetIntegerDymArrayValues(comp->did, (int)intIndex, &tp->ip[ptIndx], (int)nvals);
		break;
	}
	case fmiTypeBoolean:
	case fmiTypeInt64:
	default:
		return FMIFatal;
	}
	return FMIOK;
}




extern const unsigned int arrayInternalValueReference[];
extern const size_t arraySizes[];

DYMOLA_STATIC FMIStatus fmi3Get(FMIComponent c, typePointer *tp, fmi3Types type, const FMIValueReference* vr, size_t nvr, size_t nval){
	Component* comp = (Component*) c;
	size_t i;
	FMIStatus stat = FMIOK;
	size_t arrayMod = 0;
	const char* sfuncVer = type == fmiTypeFloat64? sFloat64: type == fmiTypeInt32? sInt32: type == fmiTypeInt64? sInt64: type == fmiTypeUInt64 ? sUInt64 :sBoolean;

	ERROR_RETURN_CHECK(FMIUpdateBeforeGet(c, sfuncVer, vr, nvr, nval));

	
	for (i = 0; i < nvr; i++) {
		fmi3Boolean isArray = fmi3False;
		const FMIValueReference r = check_arrayVR(vr[i], &isArray);
		int ix = FMI_INDEX(r);
		size_t arraySize = isArray? arraySizes[FMI_INDEX(vr[i])]:1;
		double* valueArray = NULL;
		double * arrayStart = NULL;
		
		if (comp->nCross && (0xfffffffe - r) <= comp->nCross) {
			unsigned int ixc = 0xfffffffe - r;
			tp->rp[i+arrayMod] = comp->crossingFunctions[ixc];
			goto log;
		}else{
			FMICategory iCat = (FMICategory) FMI_CATEGORY(r);
			if (iCat == fmiDynArray) {
				size_t ptIndx = i + arrayMod;
				fmi3GetDymArray(comp, tp, type, ptIndx, ix, (int)comp->dynArraySize.dim[ix]);
				arrayMod += (size_t)(comp->dynArraySize.dim[ix] - 1);
				goto log;
			}else if (iCat == fmiSystemVar2) {
				tp->rp[i + arrayMod] = comp->storeResult;
				goto log;
			}
			else if (iCat == fmiSystemVar && 0xffffff == ix) {
				tp->rp[i + arrayMod] = comp->time;
				goto log;
			}

			valueArray = getCatPointer(comp, iCat);
			if(NULL == valueArray){
				util_logger(comp, comp->instanceName, FMIError, loggFuncCall,
					"fmiGet%s: #%u# is not a valid valueReference", sfuncVer, vr[i]);
				return util_error(comp);
			}
		}

		arrayStart = &valueArray[ix];
		if(type == fmiTypeFloat64 && arraySize>1){
			memcpy(&tp->rp[i+arrayMod], arrayStart, sizeof(double)*arraySize);
		}else{
			size_t j = 0;
			for(j=0; j< arraySize; ++j){
				switch (type)
				{
				case fmiTypeFloat64:
					tp->rp[i+arrayMod+j] = arrayStart[j];
					break;
				case fmiTypeInt32:
					tp->ip[i+arrayMod+j] = (fmi3Int32) arrayStart[j];
					break;
				case fmiTypeInt64:
					tp->i64p[i+arrayMod+j] = (fmi3Int64) arrayStart[j];
					break;
				case fmiTypeUInt64:
					tp->ui64p[i + arrayMod + j] = (fmi3UInt64) arrayStart[j];
					break;
				case fmiTypeBoolean:
					tp->bp[i+arrayMod+j] = (fmi3Boolean) arrayStart[j];
					break;
				default:
					return FMIFatal;
				}
			}
		}
		arrayMod+=arraySize-1;
log:
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,"fmiGet%s: #%u#",sfuncVer, r);
	}		
	return FMIOK;
}

#define FMI_GET3(type, label, vr, nvr, value) {                                     \
	Component* comp = (Component*) instance;                                        \
	size_t i =0;                                                                    \
	for (i = 0; i < nvr; i++) {                                                     \
		const FMIValueReference r = vr[i];                                          \
		int ix = FMI_INDEX(r);                                                      \
                                                                                    \
		switch (FMI_CATEGORY(r)) {                                                  \
		case fmiOutput:                                                              \
			value[i] = (type)comp->outputs[ix];                                     \
			break;                                                                  \
        case fmiAux:                                                                 \
			value[i] = (type)comp->auxiliary[ix];                                   \
			break;                                                                  \
		case fmiState:                                                               \
			value[i] = (type)comp->states[ix];                                      \
			break;                                                                  \
		case fmiDerivative:                                                          \
			value[i] = (type)comp->derivatives[ix];                                 \
			break;                                                                  \
		case fmiParameter2:case fmiParameter:                                         \
			value[i] = (type)comp->parameters[ix];                                  \
			break;                                                                  \
        case fmiInput: case fmiInput2:                                                \
			value[i] = (type)comp->inputs[ix];                                      \
			break;                                                                  \
		case 11: /*dState */                                                        \
		    value[i] = (type)comp->dStates[ix];                                     \
			break;                                                                  \
		case 12: /*PreviousVar*/                                                    \
		    value[i] = (type)comp->dStates[ix];                                     \
			break;                                                                  \
		case dsSystemVar:                                                           \
             if( 0xffffffff == r){                                                  \
	           value[i] = (type)comp->time;                                         \
               break;                                                               \
             }                                                                      \
		default:                                                                    \
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,                \
			"fmiGet"#label": cannot get #%u#", r);                                  \
			return util_error(comp);                                                \
		}                                                                           \
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,                  \
		"fmiGet"#label": #%u# = %g", r, (double) value[i]);                         \
	}}                                                                              \




DYMOLA_STATIC FMIStatus fmi3GetFloat64_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Float64 values[],
										size_t nValues)
{
	typePointer tp;
	tp.rp = values;
	return fmi3Get(instance, &tp, fmiTypeFloat64, valueReferences, nValueReferences, nValues);
	/*
	static const FMIString fname = "Float64";
	FMIStatus stat = FMIUpdateBeforeGet(instance, fname, valueReferences, nValueReferences, nValues);
	if(stat!=FMIOK) return stat;
	FMI_GET3(fmi3Float64, Float64, valueReferences, nValueReferences, values);
	return FMIOK;
	*/
}

DYMOLA_STATIC FMIStatus fmi3GetInt32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Int32 values[],
										size_t nValues)
{
	typePointer tp;
	tp.ip = values;
	return fmi3Get(instance, &tp, fmiTypeInt32, valueReferences, nValueReferences, nValues);
	/*
	static const FMIString fname = "Int32";
	FMIStatus stat = FMIUpdateBeforeGet(instance, fname, valueReferences, nValueReferences, nValues);
	if(stat!=FMIOK) return stat;
	FMI_GET3(fmi3Int32, Int32, valueReferences, nValueReferences, values);
	return FMIOK;*/
}

DYMOLA_STATIC FMIStatus fmi3GetInt64_(fmi3Instance instance,
	const fmi3ValueReference valueReferences[],
	size_t nValueReferences,
	fmi3Int64 values[],
	size_t nValues)
{
	typePointer tp;
	tp.i64p = values;
	return fmi3Get(instance, &tp, fmiTypeInt64, valueReferences, nValueReferences, nValues);
}


DYMOLA_STATIC FMIStatus fmi3GetBoolean_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Boolean values[],
										size_t nValues)
{
	typePointer tp;
	tp.bp = values;
	return fmi3Get(instance, &tp, fmiTypeBoolean, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3GetString_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3String values[],
										size_t nValues)
{
	static const FMIString fname = "String";
	size_t i = 0;
	Component * comp = (Component*) instance;
	size_t arrayMod = 0;
	ERROR_RETURN_CHECK(FMIUpdateBeforeGet(instance, fname, valueReferences, nValueReferences, nValues))

	for ( i = 0; i < nValueReferences; i++) {
		FMIBoolean isArray = FMIFalse;
		const FMIValueReference r = check_arrayVR(valueReferences[i], &isArray);
		int ix = FMI_INDEX(r);
		size_t arraySize = isArray? arraySizes[FMI_INDEX(valueReferences[i])]:1;
		size_t j = 0;
		switch (FMI_CATEGORY(r)) {
			case fmiStringParameter:case fmiStringParameter2:
			for(j = 0; j < arraySize; ++j){
				values[i+arrayMod+j] = comp->sParameters[ix+j];
			}
			break;
		default:
			util_logger(comp, comp->instanceName, FMIError, loggBadCall, 
				"%fmi3GetString: cannot get #%u#",r);
			return util_error(comp);
		}
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,
			"%fmi3GetString: #%u# = %s", r,  values[i]);
		arrayMod+=arraySize-1;
	}
	return FMIOK;
}



DYMOLA_STATIC FMIStatus fmi3GetFloat32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Float32 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3GetFloat32");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetInt8_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Int8 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3Int8");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetUInt8_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3UInt8 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt8");
	return util_error((Component*)instance);
}


DYMOLA_STATIC FMIStatus fmi3GetInt16_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Int16 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3Int16");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetUInt16_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3UInt16 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt16");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetUInt32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3UInt32 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt32");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetUInt64_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3UInt64 values[],
										size_t nValues)
{
	typePointer tp;
	tp.ui64p = values;
	return fmi3Get(instance, &tp, fmiTypeUInt64, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3GetBinary_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										size_t valueSizes[],
										fmi3Binary values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3Binary");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetClock_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										fmi3Clock values[])
{
	unSuportedFunctionCall(instance, "fmi3Clock");
	return util_error((Component*)instance);

}
#endif
#if (FMI_VERSION < FMI_3)
/* Common get macro */
#define FMI_GET(type, label, vr_label, vr, nvr, value, idemand) {                   \
	Component* comp = (Component*) c;                                               \
	size_t i;                                                                       \
                                                                                    \
	if(comp->mStatus == modelInstantiated ){                                        \
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,                \
		"fmiGet"#label": Not allowed before call of %s", initFuncName);             \
		return util_error(comp);                                                    \
	}                                                                               \
	                                                                                \
    if (comp->mStatus != modelTerminated &&                                         \
        ( comp->icall < idemand && comp->mStatus != modelInitializationMode||       \
	    comp->mStatus == modelInitializationMode && comp->valWasSet                 \
        || comp->enforceRefresh || comp->fmi2ComputeInit )) {                       \
                                                                                    \
		 /* refresh cache */                                                        \
		int QiErr = 0;                                                              \
		if(comp->enforceRefresh && idemand <1) idemand = 3;                         \
        QiErr = util_refresh_cache(comp, idemand, NULL, NULL);                      \
		if (QiErr != 0) {                                                           \
		    FMIStatus status =                                                      \
			(QiErr == 1 && strcmp(#label, "Real") == 0) ? FMIDiscard : FMIError;    \
			util_logger(comp, comp->instanceName, status, loggQiErr,                \
			       "fmiGet"#label": dsblock_ failed, QiErr = %d", QiErr);           \
				comp->icall = 0;                                                    \
				comp->valWasSet = 1;                                                \
			HANDLE_STATUS_RETURN(status);                                           \
		}                                                                           \
	}                                                                               \
                                                                                    \
	for (i = 0; i < nvr; i++) {                                                     \
		const FMIValueReference r = vr[i];                                          \
		int ix = FMI_INDEX(r);                                                      \
                                                                                    \
		switch (FMI_CATEGORY(r)) {                                                  \
		case fmiOutput:                                                             \
			value[i] = (type)comp->outputs[ix];                                     \
			break;                                                                  \
        case fmiAux:                                                                \
			value[i] = (type)comp->auxiliary[ix];                                   \
			break;                                                                  \
		case fmiState:                                                              \
			value[i] = (type)comp->states[ix];                                      \
			break;                                                                  \
		case fmiDerivative:                                                         \
			value[i] = (type)comp->derivatives[ix];                                 \
			break;                                                                  \
		case fmiParameter2:case fmiParameter:                                       \
			value[i] = (type)comp->parameters[ix];                                  \
			break;                                                                  \
        case fmiInput: case fmiInput2:                                              \
			value[i] = (type)comp->inputs[ix];                                      \
			break;                                                                  \
		case 11: /*dState */                                                        \
		    value[i] = (type)comp->dStates[ix];                                     \
			break;                                                                  \
		case 12: /*PreviousVar*/                                                    \
		    value[i] = (type)comp->dStates[ix];                                     \
			break;                                                                  \
		case fmiSystemVar:                                                          \
             if(0xffffff == ix){                                                    \
	           value[i] = (type)comp->time;                                         \
             }                                                                      \
             break;																    \
		case fmiSystemVar2:                                                         \
              value[i] = comp->storeResult;                                         \
              break;                                                                \
		case fmiResidual:                                                           \
			value[i] = (type)comp->derivatives[comp->nxOrig + ix];                  \
			break;                                                                  \
		default:                                                                    \
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,                \
			"fmiGet"#label": cannot get #%u#", r);                                  \
			return util_error(comp);                                                \
		}                                                                           \
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,                  \
		"fmiGet"#label": #"#vr_label"%u# = %g", r, (double) value[i]);              \
	}                                                                               \
} return FMIOK




/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetReal_(FMIComponent c, const FMIValueReference vr[], size_t nvr, FMIReal value[])
{
	int idemand = getLargestIdemand(vr, nvr);
	FMI_GET(FMIReal, Real, r, vr, nvr, value, idemand);

}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetInteger_(FMIComponent c, const FMIValueReference vr[], size_t nvr, FMIInteger value[])
{
	int idemand = getLargestIdemand(vr, nvr);
	FMI_GET(FMIInteger, Integer, i, vr, nvr, value, idemand);
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetBoolean_(FMIComponent c, const FMIValueReference vr[], size_t nvr, FMIBoolean value[])
{
	int idemand = getLargestIdemand(vr, nvr);
	FMI_GET(FMIBoolean, Boolean, b, vr, nvr, value, idemand);
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetString_(FMIComponent c, const FMIValueReference vr[], size_t nvr, FMIString value[])
{
#if (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2GetString";
#else
	static FMIString label = "fmiGetString";
#endif
	Component* comp = (Component*) c;
	int idemand = getLargestIdemand(vr, nvr);
	size_t i;
	if(comp->mStatus == modelInstantiated ){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,
			"%s: Not allowed before call of %s",label, initFuncName);
		return util_error(comp);
	}
	if (comp->icall < idemand || comp->mStatus == modelInitializationMode && comp->valWasSet  || comp->fmi2ComputeInit) {
		int QiErr = 0;                                                             
        QiErr = util_refresh_cache(comp, idemand, NULL, NULL);
		if (QiErr != 0) {
			util_logger(comp, comp->instanceName, FMIError, loggQiErr,
				"%s: dsblock_ failed, QiErr = %d",label, QiErr);
			return util_error(comp);
		}
	}
	for ( i = 0; i < nvr; i++) {
		const FMIValueReference r = vr[i];
		int ix = FMI_INDEX(r);
		switch (FMI_CATEGORY(r)) {
		case fmiStringParameter2:case fmiStringParameter:
			value[i] = comp->sParameters[ix];
			break;
		default:
			util_logger(comp, comp->instanceName, FMIError, loggBadCall, 
				"%s: cannot get #s%u#",label, r);
			return util_error(comp);
		}
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,
			"%s: #s%u# = %s",label, r, (FMIString) value[i]);
	}

	return FMIOK;
}
#endif
#ifdef ONLY_INCLUDE_INLINE_INTEGRATION
#define InputSmoother_SetInput(a, b, c) (FMIFalse)
#define integration_sync_extrapolation_input(a, b)
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */

#if (FMI_VERSION >= FMI_3)

#define CHECK_MODIFIED(v1) \
	if ((v1) != value) {\
      (v1) = value;\
	  modified = FMITrue;}
					
extern void SetRealDymArrayValues(struct DYNInstanceData* did_, int index, const double* data, int nbrData);
extern void SetIntegerDymArrayValues(struct DYNInstanceData* did_, int index, const int* data, int nbrData);
extern void SetStringDymArrayValues(struct DYNInstanceData* did_, int index, const char** data, int nbrData);

DYMOLA_STATIC FMIStatus fmi3SetDymArray(FMIComponent c, ctypePointer* tp, fmi3Types type, size_t ptIndx, size_t index, size_t nvals) {
	Component* comp = (Component*)c;

	switch (type)
	{
	case fmiTypeFloat64:
		SetRealDymArrayValues(comp->did, (int)index, &tp->rp[ptIndx], (int)nvals);
		break;
	case fmiTypeInt32:
	{
		size_t intIndex = index - comp->dynArraySize.nbrReal;
		SetIntegerDymArrayValues(comp->did, (int)intIndex, &tp->ip[ptIndx], (int)nvals);
		break;
	}
	case fmiTypeBoolean:
	case fmiTypeInt64:
	case fmiTypeUInt64:
	default:
		return FMIFatal;
	}
	return FMIOK;
}

DYMOLA_STATIC FMIStatus fmi3Set(FMIComponent c, ctypePointer *tp, fmi3Types type, const FMIValueReference* vr, size_t nvr, size_t nvals){
	static const char* set= "fmi3Set";
	Component* comp = (Component*) c;
	size_t i, arrayMod = 0;
	FMIBoolean allowed = FMITrue;
	FMIBoolean anyModifiedValue = FMIFalse;
	const char* sfuncVer = type == fmiTypeFloat64? sFloat64: type == fmiTypeInt32? sInt32: type == fmiTypeUInt64? sUInt64: sBoolean;

	ERROR_RETURN_CHECK(checkSizes(comp, vr,  nvr, nvals, sfuncVer, set));

	for (i = 0; i < nvr; i++) {
		FMIBoolean isArray = FMIFalse;
		const FMIValueReference r = check_arrayVR(vr[i], &isArray);
		int ix = FMI_INDEX(r);
		FMICategory iCat = (FMICategory) FMI_CATEGORY(r);
		size_t arraySize = isArray? arraySizes[FMI_INDEX(vr[i])] : 1;
		double * valueArray = NULL;
		FMIBoolean modified = FMIFalse;
		double * arrayStart = NULL;
		if (0xffffffff == r) {
			allowed = FMIFalse;
			goto notAllowed;
		}
		else if (comp->nCross && (0xfffffffe - r) <= comp->nCross) {
			allowed = FMIFalse;
			goto notAllowed;
		}
		
		if (iCat == fmiDynArray) {
			size_t ptIndx = i + arrayMod;
			fmi3SetDymArray(comp, tp, type, ptIndx, ix, (int) comp->dynArraySize.dim[ix]);
			arrayMod += (size_t) (comp->dynArraySize.dim[ix] -1);
			continue;
		}else if (iCat == fmiSystemVar2) {
			 comp->storeResult = tp->bp[i + arrayMod];
			continue;
		}
		valueArray = getCatPointer(comp, iCat);
		if(NULL == valueArray){
			util_logger(comp, comp->instanceName, FMIError, loggFuncCall,
				"fmiGet%s: #%u# is not a valid valueReference", sfuncVer, vr[i]);
			return util_error(comp);
		}
		allowed = setIsAllowed(comp, iCat,ix);
		if(!allowed){
notAllowed:
			util_logger(comp, comp->instanceName, FMIError, loggFuncCall,
				"fmiGet%s: #%u# valueReference is not allowed to be set in current state", sfuncVer, vr[i]);
			return util_error(comp);
		}

		if (iCat == fmiInput && type == fmiTypeFloat64 && comp->useInputSmoother && comp->isCoSim) {
			if (InputSmoother_SetInput(comp, ix, tp->rp[i + arrayMod])) {
				continue;
			}
		}
		arrayStart = &valueArray[ix];
		if(type == fmiTypeFloat64 && arraySize > 1){
			size_t j = 0;
			for( ; j < arraySize; ++j){
				if(arrayStart[j] != tp->rp[i+arrayMod+j]){
					modified = FMITrue;
					break;
				}
			}
			if(modified)
				memcpy(arrayStart, &tp->rp[i+arrayMod], sizeof(double)*arraySize);
		}else{
			size_t j = 0;
			double value;
			for(; j< arraySize; ++j){
				switch (type)
				{
				case fmiTypeFloat64:
					value = tp->rp[i+arrayMod+j];
					break;
				case fmiTypeInt32:
					value = (double) tp->ip[i+arrayMod+j];
					break;
				case fmiTypeInt64:
					value = (double) tp->i64p[i+arrayMod+j];
					break;
				case fmiTypeUInt64:
					value = (double) tp->ui64p[i+arrayMod+j];
					break;
				case fmiTypeBoolean:
					value = (double) tp->bp[i+arrayMod+j];
					break;
				default:
					return FMIFatal;
				}
				if (arrayStart[j] != value) {
					arrayStart[j] = value;
					modified = FMITrue;
				}
			}
		}
		arrayMod+=arraySize-1;
		if(modified){
			anyModifiedValue = FMITrue;
			switch (iCat)
			{
			case fmiInput:
				if (comp->isCoSim)	comp->hycosimInputEvent =1;
				break;
			case fmiParameter:
				if (comp->istruct) comp->istruct->mParametersNr++;
				break;
			default:
				break;
			}			
		}
	}
    if (anyModifiedValue == FMITrue) {
   	   comp->icall = iDemandStart;
	   comp->valWasSet = 1;
	   comp->reinitCvode = 1;
	   comp->recalJacobian = 1;
	}
	return FMIOK;
}

DYMOLA_STATIC FMIStatus fmi3SetFloat64_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Float64 values[],
										size_t nValues){
	ctypePointer tp;
	tp.rp = values;
	return fmi3Set(instance,&tp,fmiTypeFloat64, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3SetInt32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Int32 values[],
										size_t nValues){
	ctypePointer tp;
	tp.ip = values;
	return fmi3Set(instance,&tp,fmiTypeInt32, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3SetInt64_(fmi3Instance instance,
	const fmi3ValueReference valueReferences[],
	size_t nValueReferences,
	const fmi3Int64 values[],
	size_t nValues)
{
	ctypePointer tp;
	tp.i64p = values;
	return fmi3Set(instance, &tp, fmiTypeInt64, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3SetBoolean_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Boolean values[],
										size_t nValues){
	ctypePointer tp;
	tp.bp = values;
	return fmi3Set(instance,&tp,fmiTypeBoolean, valueReferences, nValueReferences, nValues);
}

DYMOLA_STATIC FMIStatus fmi3SetFloat32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Float32 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3SetFloat32");
	return util_error((Component*)instance);
}


DYMOLA_STATIC FMIStatus fmi3SetInt8_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Int8 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3Int8");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetUInt8_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3UInt8 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt8");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetInt16_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3Int16 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3Int16");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetUInt16_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3UInt16 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt16");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetUInt32_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3UInt32 values[],
										size_t nValues)
{
	unSuportedFunctionCall(instance, "fmi3UInt32");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetUInt64_(fmi3Instance instance,
										const fmi3ValueReference valueReferences[],
										size_t nValueReferences,
										const fmi3UInt64 values[],
										size_t nValues)
{
	ctypePointer tp;
	tp.ui64p = values;
	return fmi3Set(instance, &tp, fmiTypeUInt64, valueReferences, nValueReferences, nValues);
}


DYMOLA_STATIC FMIStatus fmi3SetBinary_(fmi3Instance instance,
									   const fmi3ValueReference valueReferences[],
									   size_t nValueReferences,
									   const size_t valueSizes[],
									   const fmi3Binary values[],
									   size_t nValues) {
    unSuportedFunctionCall(instance, "fmi3SetBinary");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetClock_(fmi3Instance instance,
									  const fmi3ValueReference valueReferences[],
									  size_t nValueReferences,
									  const fmi3Clock values[]) {
    unSuportedFunctionCall(instance, "fmi3SetClock");
	return util_error((Component*)instance);
}


/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmi3SetString_(FMIComponent c, const FMIValueReference vr[], size_t nvr, const FMIString  value[], size_t nvals)
{
	static FMIString label = "String";
	static FMIString set = "fmi3Set";
	Component* comp = (Component*) c;
	size_t i, j;
	size_t arrayMod = 0;
	FMIBoolean allowed = FMITrue; 
	FMIBoolean modified = FMIFalse; 
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
	ERROR_RETURN_CHECK(checkSizes(comp, vr, nvr, nvals, label, set))
	for (i = 0; i < nvr; i++) {
		size_t len;
		fmi3Boolean isArray = FMIFalse;
		const FMIValueReference r = check_arrayVR(vr[i], &isArray);
		FMICategory iCat = (FMICategory) FMI_CATEGORY(r);
		unsigned int ix = FMI_INDEX(r);
		size_t arraySize = isArray? arraySizes[FMI_INDEX(vr[i])] : 1;
		switch (iCat) { 
		case fmiStringParameter:
			for(j = 0; j < arraySize; ++j){
				if(strcmp(value[i+arrayMod+j],comp->sParameters[ix+j])){
					modified = FMITrue;
					len=strlen(value[i+arrayMod+j]);
					if (len>maxStringSize) len=maxStringSize;
					memcpy((comp->sParameters)[ix+j], value[i+arrayMod+j], (len+1)*sizeof(FMIChar));
					comp->sParameters[ix+j][maxStringSize]='\0';
					if (comp->istruct) comp->istruct->mParametersNr++;
				}
			}
			break;
			
		default:
			util_logger(comp, comp->instanceName, FMIError, loggBadCall,
				"fmi3Set%s: cannot set #s%u#  ",label, r);
			return util_error(comp);
		}
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,
			"fmi3Set%s: #%u# = %s",label, r, value[i]);
	}
	if (modified == FMITrue) {
		comp->icall = iDemandStart;
		comp->valWasSet = 1;
		comp->reinitCvode = 1;
		comp->recalJacobian = 1;
	}
	return FMIOK;
}

#else
/* ---------------------------------------------------------------------- */
/* Common set macro */
#define FMI_SET(label, vr_label, m, vr, nvr, value)                            \
    Component* comp = (Component*) c;                                          \
	size_t i;                                                                  \
    FMIBoolean allowed = FMITrue;                                              \
    FMIBoolean modified = FMIFalse;                                            \
                                                                               \
	for (i = 0; i < nvr; i++) {                                                \
		const FMIValueReference r = vr[i];                                     \
		size_t ix = FMI_INDEX(r);                                              \
                                                                               \
		switch (FMI_CATEGORY(r)) {                                             \
        case fmiInput:                                                          \
			if (comp->useInputSmoother && comp->isCoSim) {				       \
				if (!InputSmoother_SetInput(comp, ix, value[i]) && comp->inputs[ix] != value[i]) { \
					comp->inputs[ix] = value[i];                               \
					modified = FMITrue;                                        \
					integration_sync_extrapolation_input(comp, ix);            \
				}                                                              \
            } else if (comp->inputs[ix] != value[i]) {                         \
			    comp->inputs[ix] = value[i];                                   \
                modified = FMITrue;                                            \
			    if (comp->isCoSim) {                                           \
				    integration_sync_extrapolation_input(comp, ix);            \
					comp->hycosimInputEvent =1;                                \
			    }                                                              \
			}                                                                  \
			break;                                                             \
		case fmiState:                                                          \
/* allow this to help our own master out, although not really legal */         \
/*		    if (comp->isCoSim && comp->mStatus != modelInstantiated) {       */\
/*                allowed = fmiFalse;                                        */\
/*			} else {                                                         */\
			    /* allow since equivalent with setContinuousStates */          \
                if (comp->states[ix] != value[i]) {                            \
				   comp->states[ix] = value[i];                                \
                   modified = FMITrue;                                         \
				}                                                              \
/*			} */                                                               \
			break;                                                             \
        case fmiAux:                                                           \
		 /*Allow if inline since states have become aux at that time*/         \
		    if ((comp->mStatus == modelInstantiated ||                         \
                comp->mStatus == modelInitializationMode ||                    \
				comp->istruct->mInlineIntegration ||                           \
                comp->AdvancedDiscreteStates) && ix >= comp->nConstAux){       \
                if (comp->auxiliary[ix] != value[i]) {                         \
			       comp->auxiliary[ix] = value[i];                             \
                   modified = FMITrue;                                         \
				}                                                              \
			}else {                                                            \
                allowed = FMIFalse;                                            \
		 	}                                                                  \
			break;                                                             \
		case fmiParameter:                                                     \
		    /* TODO: check extra bit in value reference whether tunable */     \
     		/*if (comp->mStatus == modelInstantiated || "tunable") {*/         \
			if (comp->parameters[ix] != value[i]) {                            \
			    comp->parameters[ix] = value[i];                               \
                modified = FMITrue;                                            \
				if (comp->istruct) comp->istruct->mParametersNr++;			   \
			}                                                                  \
			/*} else {*/                                                       \
            /*    allowed = fmiFalse;*/                                        \
			/*}*/															   \
			break;                                                             \
        case fmiDerivative:                                                    \
		if (comp->mStatus == modelInstantiated ||                              \
                comp->mStatus == modelInitializationMode){                     \
                if (comp->derivatives[ix] != value[i]) {                       \
			       comp->derivatives[ix] = value[i];                           \
                   modified = FMITrue;                                         \
				}                                                              \
			} else {                                                           \
                allowed = FMIFalse;                                            \
			}                                                                  \
			break;                                                             \
        case fmiOutput:                                                        \
     		if (comp->mStatus == modelInstantiated ||                          \
                comp->mStatus == modelInitializationMode){                     \
                if (comp->outputs[ix] != value[i]) {                           \
				   comp->outputs[ix] = value[i];                               \
                   modified = FMITrue;                                         \
				}                                                              \
			} else {                                                           \
                allowed = FMIFalse;                                            \
			}                                                                  \
			break;                                                             \
		case fmiDState: /*dState */                                            \
		    if (comp->dStates[ix] != value[i]) {                               \
				comp->dStates[ix] = value[i];                                  \
                modified = FMITrue;                                            \
		    }                                                                  \
			break;                                                             \
		case fmiPrevious: /*PreviousVar*/                                      \
			if (comp->dStates[ix] != value[i]) {                               \
				comp->dStates[ix] = value[i]; /*Set the state is correct*/     \
                modified = FMITrue;                                            \
		    }                                                                  \
			break;                                                             \
        case fmiSystemVar2:                                                    \
            comp->storeResult = (FMIBoolean) value[i];                         \
            break;                                                             \
	    case fmiParameter2:                                                    \
        case fmiInput2:                                                        \
			allowed = FMIFalse;                                                \
            break;                                                             \
		default:                                                               \
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,           \
		  "fmiSet"#label": cannot set #"#vr_label"%u#  ", r);                  \
		  return util_error(comp);                                             \
		}                                                                      \
                                                                               \
		if (allowed == FMIFalse) {                                             \
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,           \
			       "fmiSet"#label": may not change #"#vr_label"%u# at this "   \
			       "stage, setting of variable ignored", r);                   \
			return util_error(comp);                                           \
		}                                                                      \
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,             \
		"fmiSet"#label": #"#vr_label"%u# = %g", r, (double) value[i]);         \
	}                                                                          \
                                                                               \
    if (modified == FMITrue) {                                                 \
   	   comp->icall = iDemandStart;                                             \
	   comp->valWasSet = 1;                                                    \
	   comp->reinitCvode = 1;                                                  \
	   comp->recalJacobian = 1;                                                \
	}                                                                          \
	return FMIOK;        

/* ---------------------------------------------------------------------- */

DYMOLA_STATIC FMIStatus fmiSetReal_(FMIComponent c, const FMIValueReference vr[], size_t nvr, const FMIReal value[])
{
	FMI_SET(Real, r, c, vr, nvr, value);
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetInteger_(FMIComponent c, const FMIValueReference vr[], size_t nvr, const FMIInteger value[])
{
	FMI_SET(Integer, i, c, vr, nvr, value);
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetBoolean_(FMIComponent c, const FMIValueReference vr[], size_t nvr, const FMIBoolean value[])
{
	FMI_SET(Boolean, b, c, vr, nvr, value);
}



/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSetString_(FMIComponent c, const FMIValueReference vr[], size_t nvr, const FMIString  value[])
{
#if (FMI_VERSION >= FMI_2)
	static FMIString label = "fmi2SetString";
#else
	static FMIString label = "fmiSetString";
#endif
	Component* comp = (Component*) c;
	size_t i; 
	FMIBoolean allowed = FMITrue; 
	FMIBoolean modified = FMIFalse; 
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
	for (i = 0; i < nvr; i++) {
		size_t len;
		const FMIValueReference r = vr[i];
		int ix = FMI_INDEX(r);
		switch (FMI_CATEGORY(r)) { 
		case fmiStringParameter:
			len=strlen(value[i]);
			if (len>maxStringSize) len=maxStringSize;
			memcpy((comp->sParameters)[ix], value[i], (len+1)*sizeof(FMIChar));
			comp->sParameters[ix][maxStringSize]='\0';
			if (comp->istruct) comp->istruct->mParametersNr++;
			break;
		default:
			util_logger(comp, comp->instanceName, FMIError, loggBadCall,
				"%s: cannot set #s%u#  ",label, r);
			return util_error(comp);
		}
		if (allowed == FMIFalse) {
			util_logger(comp, comp->instanceName, FMIError, loggBadCall,
				"%s: may not change #s%u# at this stage",label, r);
			return util_error(comp);
		}
		util_logger(comp, comp->instanceName, FMIOK, loggFuncCall,
			"%s: #s%u# = %s",label, r, (FMIString) value[i]);
	}
	if (modified == FMITrue) {
		comp->icall = iDemandStart;
		comp->valWasSet = 1;
		comp->reinitCvode = 1;
		comp->recalJacobian = 1;
	}
	return FMIOK;
}
#endif

#if (FMI_VERSION >= FMI_3)

DYMOLA_STATIC FMIStatus fmi3GetNumberOfVariableDependencies_(fmi3Instance instance,
                                                           fmi3ValueReference valueReference,
                                                           size_t* nDependencies){
	unSuportedFunctionCall(instance, "fmi3GetNumberOfVariableDependencies");
	return util_error((Component*)instance);
}
/* end::GetNumberOfVariableDependencies[] */

/* tag::GetVariableDependencies[] */
DYMOLA_STATIC FMIStatus fmi3GetVariableDependencies_(fmi3Instance instance,
                                                   fmi3ValueReference dependent,
                                                   size_t elementIndicesOfDependent[],
                                                   fmi3ValueReference independents[],
                                                   size_t elementIndicesOfIndependents[],
                                                   fmi3DependencyKind dependencyKinds[],
                                                   size_t nDependencies){
	unSuportedFunctionCall(instance, "fmi3GetVariableDependencies");
	return util_error((Component*)instance);
}

#endif


#if (FMI_VERSION >= FMI_2)
extern size_t dyn_allowMultipleInstances;
extern size_t dyn_instanceDataSize;
DYMOLA_STATIC void DYNInitializeDid(struct DYNInstanceData*did_);

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC void getDelayStruct(struct DYNInstanceData* did_, delayStruct** del, size_t* nbrDel);

DYMOLA_STATIC void DYNGetTempDataReference(struct DYNInstanceData**didptr);

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiFreeFMUstate_(FMIComponent c, FMIFMUstate* FMUstate)
{

	static FMIString label = "fmi2FreeFMUstate";
	Component* comp = (Component*) c;
	Component* target;
	int i;

	if (FMUstate == NULL || *FMUstate == NULL) {
		return FMIOK;
	}
	target = (Component*) *FMUstate;

	/* first handle part common for both Model exchange and co-simulation */
	FREE_MEMORY(target->states);
	FREE_MEMORY(target->oldStates);
	FREE_MEMORY(target->derivatives);
	FREE_MEMORY(target->parameters);
	FREE_MEMORY(target->inputs);
	FREE_MEMORY(target->outputs);
	FREE_MEMORY(target->auxiliary);
	FREE_MEMORY(target->crossingFunctions);
	FREE_MEMORY(target->statesNominal);
	FREE_MEMORY(target->dStates);
	FREE_MEMORY(target->previousVars);
	if (target->basetype) {
		FREE_MEMORY(target->basetype);
	}
	for(i = (int) comp->nSPar-1; i >=0; --i){
		FREE_MEMORY( (target->sParameters)[i]);
	}
	FREE_MEMORY(target->sParameters);
	FREE_MEMORY(target->dstruct);
	FREE_MEMORY(target->istruct);
	if (target->DAEDerivatives != NULL) {
		FREE_MEMORY(target->DAEDerivatives);
	}
	if(target->did){
		delayStruct *del = NULL;
		size_t nDel;
		getDelayStruct(target->did, &del, &nDel);
		freeDelay(del, nDel);
	}
	FREE_MEMORY(target->did);

#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
	FREE_MEMORY(target->serializedDelayData);
#endif
	/* then handle co-simulation specific part */
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
	if (comp->isCoSim && !comp->istruct->mInlineIntegration) {
		if(integration_free_state(comp, target) == integrationFatal)
			return FMIFatal;
	}
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */
	FREE_MEMORY(*FMUstate);
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);
	return FMIOK;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiGetFMUstate_(FMIComponent c, FMIFMUstate* FMUstate)
{
	static FMIString label = "fmi2GetFMUstate";
	Component* source = (Component*) c;
	Component* target;
	FMIStatus status = FMIOK;

	/* first handle part common for both Model exchange and co-simulation */

	/* TODO support for many Strings */
	int freeIndex = 0;
	if(source->storeResult){
		util_logger(source, source->instanceName, FMIError, loggBadCall,"%s failed, FMU uses internal result generation; this is not possible to combine with %s",label,label); 
		return util_error(source);
	}
	util_logger(source, source->instanceName, FMIOK, loggFuncCall, label);

	if (*FMUstate == NULL) {
		FMIStatus status = allocateFMUState(FMUstate, source);
		if (status != FMIOK) {
			return status;
		}
	} 

	{
		/*Check tat del is allocated to avoid messy problems later*/
		delayStruct * del =NULL;
		size_t nDel;
		extern int Buffersize;
		getDelayStruct(source->did, &del, &nDel);
		if(del[0].nx == 0)
			allocDelayBuffers(del, nDel, Buffersize);
	}
	target = (Component*) *FMUstate;
	{
		int ret;
		switch (ret = copyVariables(source, target)){
			case 0:
				break;
			case 1:
				util_logger(source, source->instanceName, FMIError, loggMemErr,"%s failed, out of memory",label); 
				return util_error(source);
			case 2:
				util_logger(source, source->instanceName, FMIError, loggUndef,"%s failed, internal mismach when copying variables",label); 
				return util_error(source);
			default:
				util_logger(source, source->instanceName, FMIError, loggUndef,"%s failed, unhandeld internal error code %d",label,ret); 
				return util_error(source);
		}		
	}

#ifndef FMU_SOURCE_CODE_EXPORT
	if (!target->maxRunTimeReached) {
		const double max_run_time = GetAdditionalReals(4);
		if (max_run_time > 0.0) {
			extern double dymosimtime2_(int action, double* tbegin);
			double currentTimerStart = 0.0;
			dymosimtime2_(0, &currentTimerStart);
			target->oneSimulationTotalTimerStart = currentTimerStart - source->oneSimulationTotalTimerStart;
		}
	}
#endif

	/* then handle co-simulation specific part */
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
	if (source->isCoSim && !source->istruct->mInlineIntegration) {
		status = integration_get_state(source, target);
		if (status == FMIOK) {
			target->allocDone = 1;
			return FMIOK;
		} else {
			util_logger(source, source->instanceName, FMIError, loggUndef, "%s failed",label);
			return util_error(source);
		}
	}
#endif

	target->allocDone = 1;
	return FMIOK;
}

/* ---------------------------------------------------------------------- */
extern void* dynExternalObjectFirst(struct DYNInstanceData* did);

DYMOLA_STATIC FMIStatus fmiSetFMUstate_(FMIComponent c, FMIFMUstate FMUstate, FMIBoolean reinitIntegration)
{
	static FMIString label = "fmi2SetFMUstate";
	Component* target = (Component*) c;
	Component* source;
	/* first handle part common for both Model exchange and co-simulation */
	if(target->storeResult){
		util_logger(target, target->instanceName, FMIError, loggBadCall,"%s failed, FMU uses internal result generation; this is not possible to combine with %s",label,label); 
		return util_error(target);
	}
	if (FMUstate == NULL) {
		util_logger(target, target->instanceName, FMIError, loggBadCall, "%s: FMUstate == NULL",label);
		return util_error(target);
	}
	source = (Component*) FMUstate;
	assert(source->allocDone);


	/* set state vectors, derivatives, etc */
	{
		int ret = 0;
		const size_t nEobj = getNbrExternalObjects();

		if(nEobj && source->mStatus == modelInstantiated && target->mStatus > modelInstantiated){
			/*We will reset the fmu to preInitalization mode destroy and clear external obj.*/
			delayBuffersCloseNew(target->did);
			delayBuffersClearExternalTables(target->did);
		}


		if (nEobj && source->mStatus > modelInstantiated  && target->mStatus == modelInstantiated)
		{
			delayBuffersClearExternalTables(target->did);
			/* reload external tables */
			util_refresh_cache(target, 0, NULL, NULL);
		}

		switch (ret = copyVariables(source, target)) {
		case 0:
			break;
		case 1:
			util_logger(source, source->instanceName, FMIError, loggMemErr, "%s failed, out of memory", label);
			return util_error(source);
		case 2:
			util_logger(source, source->instanceName, FMIError, loggUndef, "%s failed, internal mismatch when copying variables", label);
			return util_error(source);
		default:
			util_logger(source, source->instanceName, FMIError, loggUndef, "%s failed, unhandled internal error code %d", label, ret);
			return util_error(source);
		}
		
	}
  
	/* then handle co-simulation specific part */
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
	if (target->isCoSim && !target->istruct->mInlineIntegration) {
		return integration_set_state(source, target, reinitIntegration);
	}
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */

#ifndef FMU_SOURCE_CODE_EXPORT
	if (!source->maxRunTimeReached) {
		const double max_run_time = GetAdditionalReals(4);
		if (max_run_time > 0.0) {
			extern double dymosimtime2_(int action, double* tbegin);
			target->oneSimulationTotalTimerStart = dymosimtime2_(1, &source->oneSimulationTotalTimerStart);
		}
	}
#endif

	util_logger(target, target->instanceName, FMIOK, loggFuncCall, "%s", label);
	return FMIOK;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSerializedFMUstateSize_(FMIComponent c, FMIFMUstate FMUstate, size_t *size)
{
#if FMI_VERSION >= FMI_3
	static FMIString label = "fmi3SerializedFMUstateSize";
#else
	static FMIString label = "fmi2SerializedFMUstateSize";
#endif
	Component* comp = (Component*) c;
	static size_t sizeRequired = 0;
	FMIStatus status = FMIOK;

	
	if(comp->storeResult){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,"%s failed, FMU uses internal result generation; this is not possible to combine with %s",label,label); 
		return util_error(comp);
	}
#if !(defined(NO_FILE) || defined(RTTWINCAT))
	/* verify input */
	if (FMUstate == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: FMUstate == NULL",label);
		return util_error(comp);
	}

	if (size == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: size == NULL",label);
		return util_error(comp);
	}

	/* cannot vary during session, so only needed to determine once */
	if (sizeRequired == 0) {
		tpl_node* tn = NULL;
		delayStruct* del;
		size_t ndel, delayBuffSize;
		Component* state = (Component*) FMUstate;
		getDelayStruct(state->did, &del, &ndel);
		delayBuffSize = del[0].nx*2;
		if(!state->serializedDelayData){
			state->serializedDelayData = (double*) ALLOCATE_MEMORY(ndel*delayBuffSize*2,sizeof(double));
			if(!state->serializedDelayData){
				util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: out of memory",label);
				return util_error(comp);
			}
		}
		DYNPropagateDidToThread(comp->did);
		EXCEPTION_TRY
			tn = createTplMap(state, comp->tplDidBin);
			if (tn == NULL) {
				status = FMIError;
				goto clean;
			}

			if (tpl_pack(tn, 0) != 0) {
				util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_pack failed",label);
				status = FMIError;
				goto clean;
			} else {
				/* fetch required size */
				if (tpl_dump(tn, TPL_GETSIZE, &sizeRequired) != 0) {
					util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_dump failed",label);
					status = FMIError;
					goto clean;
				}
			}
	clean:
			tpl_free(tn); tn = NULL;
		EXCEPTION_CATCH_ALL;
			util_logger(comp, comp->instanceName, FMIFatal, loggUndef, "%s: Fatal Error with tpl",label);
			return FMIFatal;
		EXCEPTION_END;

		FREE_MEMORY(state->serializedDelayData);
		if(status == FMIError)
			return util_error(comp);
	}

	*size = sizeRequired;
#else
	util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: FMU is compiled with flag NO_FILE indicating that the running system does not have a file system,\r\n use fmi2Get/SetFMUstate instead",label);
	status = FMIError;
#endif
	return status;
}

/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiSerializeFMUstate_(FMIComponent c,
											  FMIFMUstate FMUstate,
											  FMIByte* serializedState,
											  size_t size)
{
#if FMI_VERSION >= FMI_3
	static FMIString label = "fmi3SerializeFMUstate";
#else
	static FMIString label = "fmi2SerializeFMUstate";
#endif
	Component* comp = (Component*) c;
	Component* state = (Component*) FMUstate;
	FMIStatus status = FMIOK;
	delayStruct * del =NULL;
	size_t delayBuffSize, nDel, i;

#if !(defined(NO_FILE) || defined(RTTWINCAT))
	tpl_node *tn;
	if(comp->storeResult){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,"%s failed, FMU uses internal result generation; this is not possible to combine with %s",label,label); 
		return util_error(comp);
	}
	/* verify input */
	if (FMUstate == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: FMUstate == NULL",label);
		return util_error(comp);
	}

	if (serializedState == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: serializedState == NULL",label);
		return util_error(comp);
	}
	getDelayStruct(state->did, &del, &nDel);
	delayBuffSize = (size_t) del[0].nx;
	
	if(!state->serializedDelayData){
		state->serializedDelayData = (double*) ALLOCATE_MEMORY(nDel*delayBuffSize*2,sizeof(double));
		if(!state->serializedDelayData){
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: out of memory",label);
			return util_error(comp);
		}
	}

	for(i = 0; i< nDel; ++i){
		memcpy(state->serializedDelayData+i*delayBuffSize*2,del[i].x,delayBuffSize*2*sizeof(double));
	}
	
	DYNPropagateDidToThread(comp->did);
	EXCEPTION_TRY
		/* setup tpl */
		tn = createTplMap(state, comp->tplDidBin);
		if (tn == NULL) {
			status = FMIError;
			goto clean;
		}

		/* save state */
		if (tpl_pack(tn, 0) != 0) {
			util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_pack failed",label);
			status = FMIError;
			goto clean;
		} else {
			/* dump to target array */
			if (tpl_dump(tn, TPL_MEM|TPL_PREALLOCD, serializedState, size) != 0) {
				util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_dump failed",label);
				status = FMIError;
				goto clean;
			}
		}
	clean:
		tpl_free(tn); tn = NULL;
	EXCEPTION_CATCH_ALL;
		util_logger(comp, comp->instanceName, FMIFatal, loggUndef, "%s: Fatal Error with tpl",label);
		return FMIFatal;
	EXCEPTION_END;


	FREE_MEMORY(state->serializedDelayData);
	if(status == FMIError)
		return util_error(comp);
#else
	util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: FMU is compiled with flag NO_FILE indicating that the running system does not have a file system,\r\n use fmi2Get/SetFMUstate instead",label);
	status = FMIError;
#endif
	return status;
}
#define STATIC_TMP_ALLOC 128
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC FMIStatus fmiDeSerializeFMUstate_(FMIComponent c, 
												const FMIByte serializedState[],
												size_t size,
												FMIFMUstate* FMUstate)
{
#if FMI_VERSION >= FMI_3
	static FMIString label = "fmi3DeSerializeFMUstate";
#else
	static FMIString label = "fmi2DeSerializeFMUstate";
#endif
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;
#if !(defined(NO_FILE) || defined(RTTWINCAT))
	tpl_node *tn;
	Component* state;
	delayStruct * del = NULL;
	size_t delayBuffSize, nDel, i;
	double ** tmpDelayBuffers = NULL;
	double * tmpDelayBuffersStatic[STATIC_TMP_ALLOC] = {0};
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
	/* verify input */
	if(comp->storeResult){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall,"%s failed, FMU uses internal result generation; this is not possible to combine with %s",label,label); 
		return util_error(comp);
	}
	if (serializedState == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: serializedState == NULL",label);
		return util_error(comp);
	}

	if (FMUstate == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "%s: FMUstate == NULL",label);
		return util_error(comp);
	}

	if(*FMUstate==NULL){ /* setup state target */
		status = allocateFMUState(FMUstate, comp);
		if (status != FMIOK) {
			return status;
		}
	}
	state = (Component*) *FMUstate;
	state->allocDone = 1;
	getDelayStruct(state->did, &del, &nDel);

	delayBuffSize = (size_t) del[0].nx;
	if(nDel > STATIC_TMP_ALLOC){
		tmpDelayBuffers = (double**) ALLOCATE_MEMORY(nDel,sizeof(double*));
		if(!tmpDelayBuffers){
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: out of memory",label);
			goto clean2;
		}
	}else{
		tmpDelayBuffers = tmpDelayBuffersStatic;
	}
	

	for(i = 0; i < nDel; ++i){
		tmpDelayBuffers[i] = del[i].x;
	}
	if(!state->serializedDelayData){
		state->serializedDelayData = (double*) ALLOCATE_MEMORY(nDel*delayBuffSize*2,sizeof(double));
		if(!state->serializedDelayData){
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: out of memory",label);
			status = FMIError;
			goto clean2;
		}
	}
	DYNPropagateDidToThread(comp->did);
	EXCEPTION_TRY
		/* setup tpl*/
		tn = createTplMap(state, comp->tplDidBin);
		if (tn == NULL) {
			return util_error(comp);
		}

		/* load from source array */
		if (tpl_load(tn, TPL_MEM, serializedState, size) != 0) {
			util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_load failed",label);
			status = FMIError;
		} else {
			/* restore state */
			size_t j;
			/* first free old string parameters memory since tpl allocates new for strings (and binary data) */
			for (j = 0; j < state->nSPar; j++) {
				FREE_MEMORY(state->sParameters[j]);
			}

			if (tpl_unpack(tn, 0) < 0) {
				util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: tpl_unpack failed", label);
				status = FMIError;
				goto clean;
			} else {
				/* restore did */
				memcpy(state->did, comp->tplDidBin->addr, comp->tplDidBin->sz);
				for (i = 0; i < nDel; ++i) {
					del[i].x = tmpDelayBuffers[i];
					memcpy(del[i].x, state->serializedDelayData + i * delayBuffSize * 2, delayBuffSize * 2 * sizeof(double));
					del[i].y = del[i].x + delayBuffSize;
				}
			}
			// tpl_unpack uses something else, so we need to replace
			for (j = 0; j < state->nSPar; j++) {
				FMIChar *sParameter = state->sParameters[j];
				if (sParameter)
				{
					state->sParameters[j] = (FMIChar*) ALLOCATE_MEMORY(maxStringSize + 1, sizeof(FMIChar));
					if(!state->sParameters[j]){
						util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: out of memory",label);
						status = FMIError;
						goto clean;
					}
					memcpy(state->sParameters[j], sParameter, (strlen(sParameter) + 1) * sizeof(FMIChar));
					free(sParameter);
				}
			}
#if 0
			/* Dymola#2049 Note: Optionally treat as new parameters, triggering re-evaluation of computed parameters */
			if (comp->istruct) comp->istruct->mParametersNr++;
#endif
		}
clean:
		if(comp->tplDidBin){
			tplFree(comp->tplDidBin->addr);
			comp->tplDidBin->addr = NULL;
		}
		tpl_free(tn); tn = NULL;
	EXCEPTION_CATCH_ALL;
		util_logger(comp, comp->instanceName, FMIFatal, loggUndef, "%s: Fatal Error with tpl",label);
		return FMIFatal;
	EXCEPTION_END;
clean2:
	FREE_MEMORY(state->serializedDelayData);
	if(nDel > STATIC_TMP_ALLOC){
		FREE_MEMORY(tmpDelayBuffers);
	}
	if(status == FMIError)
		return util_error(comp);
	return status;
#else
	util_logger(comp, comp->instanceName, FMIError, loggUndef, "%s: FMU is compiled with flag NO_FILE indicating that the running system does not have a file system,\r\n use fmi2Get/SetFMUstate instead",label);
	return util_error(comp);
#endif
}

/* Return 0 on success */
static int DYNAllocateJacobian(int nx, int nu, int ny, JacobianData* jacData) {
	int nA, nB, nC, nD;
	nA = nx * nx;
	nB = nx * nu;
	nC = ny * nx;
	nD = ny * nu;
	if (jacData==NULL) return 2;

	/* avoid re-allocation on each call */
	if (jacData->nJacA < nA || jacData->jacA == NULL) {
		FREE_MEMORY(jacData->jacA);
		jacData->jacA = (double*) ALLOCATE_MEMORY(nA+1, sizeof(double));
	}
	if (jacData->nJacB < nB || jacData->jacB == NULL) {
		FREE_MEMORY(jacData->jacB);
		jacData->jacB = (double*) ALLOCATE_MEMORY(nB+1, sizeof(double));
	}
	if (jacData->nJacC < nC || jacData->jacC == NULL) {
		FREE_MEMORY(jacData->jacC);
		jacData->jacC = (double*) ALLOCATE_MEMORY(nC+1, sizeof(double));
	}
	if (jacData->nJacD < nD || jacData->jacD == NULL) {
		FREE_MEMORY(jacData->jacD);
		jacData->jacD = (double*) ALLOCATE_MEMORY(nD+1, sizeof(double));
	}

	if (jacData->jacA == NULL || jacData->jacB == NULL || jacData->jacC== NULL || jacData->jacD == NULL) {
		return 1;
	}

	jacData->nJacA = nA;
	jacData->nJacB = nB;
	jacData->nJacC = nC;
	jacData->nJacD = nD;
	return 0;
}
/* Return 0 on success */
/* Important note: It does not shrink */
static int DYNAllocateDirHelper(size_t nv, size_t nz, JacobianData* jacData) {
	if (jacData==NULL) return 2;
	if (jacData->nJacV < nv) {
		FREE_MEMORY(jacData->jacV);
		FREE_MEMORY(jacData->jacVTmp1);
		FREE_MEMORY(jacData->jacVTmp2);
		jacData->jacV = (double*) ALLOCATE_MEMORY(nv, sizeof(double));
		jacData->jacVTmp1 = (double*) ALLOCATE_MEMORY(nv, sizeof(double));
		jacData->jacVTmp2 = (double*) ALLOCATE_MEMORY(nv, sizeof(double));
		if (jacData->jacV == NULL || jacData->jacVTmp1 == NULL || jacData->jacVTmp2 == NULL) return 1;
		jacData->nJacV = nv;
	}
	if (jacData->nJacZ < nz) {
		FREE_MEMORY(jacData->jacZ);
		FREE_MEMORY(jacData->jacZTmp1);
		FREE_MEMORY(jacData->jacZTmp2);
		jacData->jacZ = (double*) ALLOCATE_MEMORY(nz, sizeof(double));
		jacData->jacZTmp1 = (double*) ALLOCATE_MEMORY(nz, sizeof(double));
		jacData->jacZTmp2 = (double*) ALLOCATE_MEMORY(nz, sizeof(double));
		if (jacData->jacZ == NULL || jacData->jacZTmp1 == NULL || jacData->jacZTmp2 == NULL) return 1;
		jacData->nJacZ = nz;
	}
	return 0;
}
static int JacInGroupI(char* updatedJacobianP1, int col, int allCols, int update) {
	if (col >= 0 && col < allCols) {
		// Ignoring errors
		if (update) updatedJacobianP1[col] = 2;
		else if (updatedJacobianP1[col] == 1) {
			return 1;
		}
	}
	return 0;
}
static int JacInGroup(char* updatedJacobianP1, int* grp, int allCols, int update) {
	int nMembers = grp[0];
	int member = 0;
	for (member = 1; member <= nMembers; member++) {
		int col = grp[member]-1;
		if (JacInGroupI(updatedJacobianP1, col, allCols, update)) return 1;
	}
	return 0;
}
static int JacInGroupS(char* updatedJacobianP1, const int* b, int allCols, int update) {
	int groupMember, column;
	for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
		for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
			int col = column - 1;
			if (JacInGroupI(updatedJacobianP1, col, allCols, update)) return 1;
		}
	}
	return 0;
}
/* ---------------------------------------------------------------------- */
DYMOLA_STATIC void SetDymolaJacobianPointers(struct DYNInstanceData*did_, double * QJacobian_,double * QBJacobian_,double * QCJacobian_,double * QDJacobian_,int QJacobianN_,
	int QJacobianNU_,int QJacobianNY_,double * QJacobianSparse_,int * QJacobianSparseR_,int * QJacobianSparseC_,int QJacobianNZ_);
DYMOLA_STATIC void SetDymolaDirPointers(struct DYNInstanceData* did_, int n, const double* QXDir_var, const double* QUDir_var, double* QDDir_var, double* QYDir_var);
DYMOLA_STATIC void SetDymolaAdjointPointers(struct DYNInstanceData* did_,
	double* QFR_var, double* QWR_var, double* QHelpR_var, double* QUR_var, double* QYR_var, double* QXR_var, double* QPR_var, double* QAux_var);
DYMOLA_STATIC void SetDymolaEnforceWhen(struct DYNInstanceData*did_, int val);
#if (FMI_VERSION >= FMI_3)
DYMOLA_STATIC FMIStatus fmi3GetDirectionalDerivative_(FMIComponent c,
                                                    const fmi3ValueReference z_ref[],
                                                    size_t nz,
                                                    const fmi3ValueReference v_ref[],
                                                    size_t nv,
                                                    const fmi3Float64 dv[],
                                                    size_t nSeed,
                                                    fmi3Float64 dz[],
                                                    size_t nSensitivity)
/*
typedef fmi3Status fmi3GetDirectionalDerivativeTYPE(fmi3Instance instance,
                                                    const fmi3ValueReference unknowns[],
                                                    size_t nUnknowns,
                                                    const fmi3ValueReference knowns[],
                                                    size_t nKnowns,
                                                    const fmi3Float64 seed[],
                                                    size_t nSeed,
                                                    fmi3Float64 sensitivity[],
                                                    size_t nSensitivity);
*/
{
	static FMIString label = "fmi3GetDirectionalDerivative";
#else
DYMOLA_STATIC FMIStatus fmiGetDirectionalDerivative_(FMIComponent c,
													 const FMIValueReference z_ref[],
													 size_t nz,
													 const FMIValueReference v_ref[],
													 size_t nv,
													 const FMIReal dv[],
													 FMIReal dz[])
{
	static FMIString label = "fmi2GetDirectionalDerivative";
#endif


	size_t i;
	size_t j;
	Component* comp = (Component*) c;
	FMIStatus status = FMIOK;

	JacobianData* jacData = &comp->jacData;
	int analyticABCD;
	int directionalABCD;
	extern int QJacobianDefined_;
	extern int QJacobianCG_[];
	extern struct QJacobianTag_ QJacobianGC2_[];
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);

	analyticABCD = (QJacobianDefined_ & 3) == 3;
	directionalABCD = (QJacobianDefined_ & 24) == 24;
#if (FMI_VERSION >= FMI_3)
	if(nSeed > nv){
		static const char* get ="";
		ERROR_RETURN_CHECK(checkSizes(comp, v_ref, nv, nSeed, label, get));
	}
	if(nSensitivity > nz){
		static const char* get ="";
		ERROR_RETURN_CHECK(checkSizes(comp, z_ref, nz, nSensitivity, label, get));
	}
	for (i = 0; i < nSensitivity; i++) {
		dz[i] = 0;
	}
#else
	for (i = 0; i < nz; i++) {
		dz[i] = 0;
	}
#endif
	/*  analytic Jacobian available - or smart new logic for grouping ? */
	/* Note the smart logic without grouping doesn't really make sense - unless the users are bad */
	if ((analyticABCD || (QJacobianCG_[0] > 0 && nv==1 && dv[0]==1)) && (comp->mStatus != modelInstantiated &&
		comp->mStatus != modelInitializationMode) && !comp->AdvancedDiscreteStates ) {
			/*Analytical Jacobians are only valid after intialization*/
			/* Assume the same for grouping etc */
		int nx, nx2, nu, ny, nw, np, nrel, ncons, dae;
		
		
		GetDimensions(&nx, &nx2, &nu, &ny, &nw, &np, &nrel, &ncons, &dae);
		FMIBoolean isFmiDAE = analyticABCD && GetAdditionalFlags(40);
		if (isFmiDAE) {
			comp->nxOrig = nx;
			nx = nx + nx2;
		}

		if (DYNAllocateJacobian(nx, nu, ny, jacData)) {
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
			status = FMIError;
			goto done1;
		}
		if (analyticABCD) {
			/* Analytic Jacobian */ 
			if(comp->icall < iDemandDerivative && comp->mStatus != modelInitializationMode || comp->recalJacobian){
				SetDymolaJacobianPointers(comp->did, jacData->jacA, jacData->jacB, jacData->jacC, jacData->jacD, nx, nu, ny, 0, 0,0,0);
				if (util_refresh_cache(comp, iDemandDerivative, label, NULL)) {
					status = FMIError;
					goto done1;
				}
				comp->recalJacobian = 0;
			}
		} else {
			/* From updateJacobians in dymosim2.c. Note that nv=1 and we ignore z */
			if (jacData->nUpdatedJacobian<nx+nu || jacData->updatedJacobian==NULL) {
				FREE_MEMORY(jacData->updatedJacobian);
				jacData->updatedJacobian=(char*)calloc(nx+nu+1,sizeof(char));
				jacData->nUpdatedJacobian=nx+nu;
			}
			if (DYNAllocateDirHelper(nx+nu, nx+ny, jacData) || jacData->updatedJacobian==NULL) {
				util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
				status = FMIError;
				goto done1;
			}
			/* Reset needed columns */
			if(comp->icall < iDemandDerivative && comp->mStatus != modelInitializationMode || comp->recalJacobian){
				i=0;
				for(i=0;i<(size_t)(nx+nu);++i) jacData->updatedJacobian[1+i]=0;
				comp->recalJacobian = 0;
			}
			/* Grouping: figure out which columns we need*/
			{
				/* Set which columns to update. */
				size_t j;
				for (j = 0; j < nv; ++j) {
#if (FMI_VERSION >= FMI_3)
					size_t k = 0;
					FMIBoolean isArray = FMIFalse;
					FMIValueReference vr = check_arrayVR(v_ref[j], &isArray);
					FMICategory dvCat = (FMICategory)((vr >> 24) & 0xf);
					size_t dvNum1 = (vr & 0xffffff);
					size_t arraySize = isArray? arraySizes[FMI_INDEX(v_ref[i])]:1;
					for(; k < arraySize; ++k){
						size_t dvNum = dvNum1+k;
#else
					FMICategory dvCat = (FMICategory)((v_ref[j] >> 24) & 0xf);
					integer dvNum = (v_ref[j] & 0xffffff);
#endif
					if (dvCat == fmiState) {
						if (jacData->updatedJacobian[1 + dvNum] == 0) jacData->updatedJacobian[1 + dvNum] = 1;
					} else if (dvCat == fmiInput) {
						if (jacData->updatedJacobian[1 + dvNum + nx] == 0) jacData->updatedJacobian[1 + dvNum + nx] = 1;
					} else {
						/* Error - ignore now*/
					}
#if (FMI_VERSION >= FMI_3)
				}
#endif
				}
			}
			/* Actual grouping code */
			/* Note that we cannot use the FMI-Get-Real interface since we don't have value references here*/
			{
				int ind=0;
				int nStandardGroups = QJacobianCG_[0];
				int fmiCGstartIndex =1;
				int nGroups = -1, groupIx = -1, groupNr = -1, grnbr = -1;

				comp->istruct->mInJacobian=1;
				for(grnbr = 0; grnbr < nStandardGroups;++grnbr){
					int nMembers = QJacobianCG_[fmiCGstartIndex];
					if (nMembers < 0) nMembers = 0;
					fmiCGstartIndex+= 1+nMembers;
				}
				nGroups = QJacobianCG_[fmiCGstartIndex];
				groupIx = fmiCGstartIndex+1;
				if (directionalABCD) {
					for (ind = 0; ind < nx+nu; ++ind) jacData->jacV[ind] = 0.0;
					for (ind = 0; ind < nx+ny; ++ind) jacData->jacZ[ind] = 0.0;
				} else {
					for (ind = 0; ind < nx; ++ind) jacData->jacV[ind] = comp->states[ind];
					for (; ind < nx + nu; ++ind)jacData->jacV[ind] = comp->inputs[ind - nx];
					for (ind = 0; ind < nx; ++ind) jacData->jacZ[ind] = comp->derivatives[ind];
					for (; ind < nx + ny; ++ind)jacData->jacZ[ind] = comp->outputs[ind - nx];
				}
				/* Iterate the groups.*/
				for (groupNr = 0; groupNr < nGroups; groupNr++) {
					const struct QJacobianTag_ pair = QJacobianGC2_[nStandardGroups + groupNr];
					const int* const b = pair.b;
					int nMembers = QJacobianCG_[groupIx];
					int structuredGroup = 0;
					if (nMembers < 0) {
						nMembers = 0;
						structuredGroup = 1;
					}
					if (structuredGroup ? JacInGroupS(jacData->updatedJacobian +1 , b, nx+nu, 0) : JacInGroup(jacData->updatedJacobian + 1, QJacobianCG_ + groupIx, nx+nu, 0)) {
						/* Something needs to be computed for group */
						if (directionalABCD) {
							/* Similar to code below */
							int member = 0;
							if (structuredGroup) {
								int groupMember, column;
								for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
									for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
										int col = column - 1;
										jacData->jacV[col] = 1;
									}
								}
							} else for (member = 1; member <= nMembers; member++) {
								/* Fetch column.*/
								int col = QJacobianCG_[groupIx + member] - 1;
								jacData->jacV[col] = 1;
							}
							{
								int idemand = 2;
								int QiErr = 0;
								comp->enforceRefresh = FMITrue;
								comp->icall = 0;
								SetDymolaDirPointers(comp->did, groupNr == 0, jacData->jacV, jacData->jacV + nx, jacData->jacZTmp1, jacData->jacZTmp1 + nx);
								QiErr = util_refresh_cache(comp, idemand, NULL, NULL);
								SetDymolaDirPointers(comp->did, 0, 0, 0, 0, 0);
								comp->enforceRefresh = FMIFalse;
								/* Not fully correct, even if we reset inputs and states we don't reset helpvars, so a subsequent call to AcceptedSection (or AdjointSection) may give wrong results */
								/* The derivatives and states are correct though - so we might do something more for that */
								if (QiErr) {
									util_logger(comp, comp->instanceName, FMIError, loggQiErr,
										"%s: dsblock_ failed, QiErr = %d", label, QiErr);
									goto done2;
								}
							}
							if (structuredGroup) {
								int groupMember, column;
								for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
									for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
										int col = column - 1;
										jacData->jacV[col] = 0;
									}
								}
							} else for (member = 1; member <= nMembers; member++) {
								/* Reset column.*/
								int col = QJacobianCG_[groupIx + member] - 1;
								jacData->jacV[col] = 0;
							}
							{
								/* Store result.*/
								int GC_new_column = -1;
								int GC_num_rows = 0;
								int GC_ind = 1;
								int col_save = 0;
								int GC_size = 0;

								if (pair.a == 0)
									GC_size = nx + ny;
								else if (pair.a == 1) {
									int i;
									GC_ind = 1;
									GC_size = 0;
									for (i = 0; i < b[0]; ++i) {
										++GC_ind;
										GC_num_rows = b[GC_ind++];
										GC_size += GC_num_rows;
										GC_ind += GC_num_rows;
									}
								} else if (pair.a == 11) {
									int groupMember, column, band;
									GC_size = 0; /* Skipping code below */
									for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
										for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
											for (band = 0; band < b[groupMember + 3]; ++band) {
												int row = b[groupMember + 4 + band * 3 + 2] + (column - b[groupMember]);
												if (row >= b[groupMember + 4 + band * 3 + 0] && row < b[groupMember + 4 + band * 3 + 1]) {
													int col = column;
													/* Same as below - use some inline trick! */
													if (col >= 1) {
														int mat = 0;
														int deltaColMod = 0;
														int jnrows = 0;
														double* jac = NULL;
														double* fPert = NULL;
														double deltasum = 0;
														if (row >= nx) {
															row -= nx;
															mat += 1 << 0;
															fPert = jacData->jacZTmp1;
															jnrows = ny;
														} else {
															fPert = jacData->jacZTmp1;
															jnrows = nx;
														}
														if (col > nx) {
															col -= nx;
															mat += 1 << 1;
															deltaColMod = nx;
														}
														--col; /*all use of col here is  cindex based so reduce col*/
														jac = mat < 2 ? (mat == 0 ? jacData->jacA : jacData->jacC) : (mat == 2 ? jacData->jacB : jacData->jacD);
														jac[row + (col)*jnrows] = fPert[row];
														jac = NULL;
														fPert = NULL;
													}
												}
											}
										}
									}
										
								} else goto done2;
								GC_ind = 1;
								GC_num_rows = 0;
								{
									int i;
									for (i = 0; i < GC_size; i++) {
										int col;
										int row;
										if (pair.a == 0) {
											col = b[i];
											row = i;
										} else if (pair.a == 1) {
											if (GC_ind >= GC_new_column + GC_num_rows + 2) {
												GC_new_column = GC_ind;
												col_save = b[GC_ind++];
												GC_num_rows = b[GC_ind++];
											}
											col = col_save;
											row = b[GC_ind++] - 1;
										} else goto done2;
										if (col >= 1) {
											int mat = 0;
											int deltaColMod = 0;
											int jnrows = 0;
											double* jac = NULL;
											double* fPert = NULL;
											double deltasum = 0;
											if (row >= nx) {
												row -= nx;
												mat += 1 << 0;
												fPert = jacData->jacZTmp1;
												jnrows = ny;
											} else {
												fPert = jacData->jacZTmp1;
												jnrows = nx;
											}
											if (col > nx) {
												col -= nx;
												mat += 1 << 1;
												deltaColMod = nx;
											}
											--col; /*all use of col here is  cindex based so reduce col*/
											jac = mat < 2 ? (mat == 0 ? jacData->jacA : jacData->jacC) : (mat == 2 ? jacData->jacB : jacData->jacD);
											jac[row + (col)*jnrows] = fPert[row];
											jac = NULL;
											fPert = NULL;
										}
									}
								}
							}
						} else {
							int sign;
							double sqrtEps = 1e-8;
							int failedPos = 1; /* For safety - start by assuming that both failed - and then set failed or not below */
							int failedNeg = 1;

							/* Using central difference why multiple iterations necessary. */
							for (sign = 1; sign >= -1; sign -= 2) {
								/* Perturb each group member.*/
								int member = 0;
								if (structuredGroup) {
									int groupMember, column;
									for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
										for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
											int col = column - 1;
											int disturbeU = col >= nx;
											int ucol = col - nx;
											/* Perturb.*/
											if (disturbeU) {
												comp->inputs[ucol] += ((comp->inputs[ucol] < 0) ? -sign : sign) * sqrtEps * Dymola_max(Dymola_abs(comp->inputs[ucol]), 1.0);
											} else {
												comp->states[col] += ((comp->states[col] < 0) ? -sign : sign) * sqrtEps * Dymola_max(Dymola_abs(comp->states[col]), 1.0);
											}
											if (sign == 1) {
												jacData->jacVTmp1[col] = (disturbeU ? comp->inputs[ucol] - jacData->jacV[col] : comp->states[col] - jacData->jacV[col]);
											} else {
												jacData->jacVTmp2[col] = (disturbeU ? comp->inputs[ucol] - jacData->jacV[col] : comp->states[col] - jacData->jacV[col]);
											}
										}
									}
								} else for (member = 1; member <= nMembers; member++) {
									/* Fetch column.*/
									int col = QJacobianCG_[groupIx + member] - 1;
									int disturbeU = col >= nx;
									int ucol = col - nx;
									/* Perturb.*/
									if (disturbeU) {
										comp->inputs[ucol] += ((comp->inputs[ucol] < 0) ? -sign : sign) * sqrtEps * Dymola_max(Dymola_abs(comp->inputs[ucol]), 1.0);
									} else {
										comp->states[col] += ((comp->states[col] < 0) ? -sign : sign) * sqrtEps * Dymola_max(Dymola_abs(comp->states[col]), 1.0);
									}
									if (sign == 1) {
										jacData->jacVTmp1[col] = (disturbeU ? comp->inputs[ucol] - jacData->jacV[col] : comp->states[col] - jacData->jacV[col]);
									} else {
										jacData->jacVTmp2[col] = (disturbeU ? comp->inputs[ucol] - jacData->jacV[col] : comp->states[col] - jacData->jacV[col]);
									}
								}

								/* Evaluate.*/
								{
									int idemand = 2;
									int QiErr = 0;
									comp->enforceRefresh = FMITrue;
									comp->icall = 0;
									QiErr = util_refresh_cache(comp, idemand, NULL, NULL);
									if (sign == 1) {
										failedPos = QiErr;
										for (ind = 0; ind < nx; ++ind) jacData->jacZTmp1[ind] = comp->derivatives[ind];
										for (; ind < nx + ny; ++ind) jacData->jacZTmp1[ind] = comp->outputs[ind - nx];
									} else {
										failedNeg = QiErr;
										for (ind = 0; ind < nx; ++ind) jacData->jacZTmp2[ind] = comp->derivatives[ind];
										for (; ind < nx + ny; ++ind) jacData->jacZTmp2[ind] = comp->outputs[ind - nx];
									}
									comp->enforceRefresh = FMIFalse;
									/* Not fully correct, even if we reset inputs and states we don't reset helpvars, so a subsequent call to AcceptedSection (or AdjointSection) may give wrong results */
									/* The derivatives and states are correct though - so we might do something more for that */
								}

								/* Reset state/input.*/
								if (structuredGroup) {
									int groupMember, column;
									for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
										for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
											int col = column - 1;
											/* Reset.*/
											int disturbeU = col >= nx;
											int ucol = col - nx;
											if (disturbeU) {
												comp->inputs[ucol] = jacData->jacV[col];
											} else {
												comp->states[col] = jacData->jacV[col];
											}
										}
									}
								} else for (member = 1; member <= nMembers; member++) {
									/* Fetch column.*/
									int col = QJacobianCG_[groupIx + member] - 1;
									/* Reset.*/
									int disturbeU = col >= nx;
									int ucol = col - nx;
									if (disturbeU) {
										comp->inputs[ucol] = jacData->jacV[col];
									} else {
										comp->states[col] = jacData->jacV[col];
									}
								}
							}
							/* And reset derivatives/outputs,  */
							for (ind = 0; ind < nx; ++ind) comp->derivatives[ind] = jacData->jacZ[ind];
							for (; ind < nx + ny; ++ind)comp->outputs[ind - nx] = jacData->jacZ[ind];
							if (failedPos && failedNeg) {
								util_logger(comp, comp->instanceName, FMIError, loggQiErr,
									"%s: dsblock_ failed, QiErr = %d", label, failedPos);
								goto done2;
							}
							{
								/* Store result.*/
								const struct QJacobianTag_ pair = QJacobianGC2_[nStandardGroups + groupNr];
								const int* const b = pair.b;
								int GC_new_column = -1;
								int GC_num_rows = 0;
								int GC_ind = 1;
								int col_save = 0;
								int GC_size = 0;

								if (pair.a == 0)
									GC_size = nx + ny;
								else if (pair.a == 1) {
									int i;
									GC_ind = 1;
									GC_size = 0;
									for (i = 0; i < b[0]; ++i) {
										++GC_ind;
										GC_num_rows = b[GC_ind++];
										GC_size += GC_num_rows;
										GC_ind += GC_num_rows;
									}
								} else if (structuredGroup && pair.a == 11) {
									GC_size = 0; /* Skipping code below */
									int groupMember, column, band;
									GC_size = 0; /* Skipping code below */
									for (groupMember = 0; b[groupMember]; groupMember += 4 + b[groupMember + 3] * 3) {
										for (column = b[groupMember]; column < b[groupMember + 2]; column += b[groupMember + 1]) {
											for (band = 0; band < b[groupMember + 3]; ++band) {
												int row = b[groupMember + 4 + band * 3 + 2] + (column - b[groupMember]);
												if (row >= b[groupMember + 4 + band * 3 + 0] && row < b[groupMember + 4 + band * 3 + 1]) {
													int col = column;
													/* Same as below - use some inline trick! */
													int mat = 0;
													int deltaColMod = 0;
													int jnrows = 0;
													double* jac = NULL;
													double* fPert = NULL;
													double* fPert2 = NULL;
													double deltasum = 0;
													if (row >= nx) {
														row -= nx;
														mat += 1 << 0;
														fPert = failedPos ? comp->outputs : jacData->jacZTmp1 + nx;
														fPert2 = failedNeg ? comp->outputs : jacData->jacZTmp2 + nx;
														jnrows = ny;
													} else {
														fPert = failedPos ? comp->derivatives : jacData->jacZTmp1;
														fPert2 = failedNeg ? comp->derivatives : jacData->jacZTmp2;
														jnrows = nx;
													}
													if (col > nx) {
														col -= nx;
														mat += 1 << 1;
														deltaColMod = nx;
													}
													--col; /*all use of col here is  cindex based so reduce col*/
													jac = mat < 2 ? (mat == 0 ? jacData->jacA : jacData->jacC) : (mat == 2 ? jacData->jacB : jacData->jacD);
													deltasum = (failedPos ? 0.0 : jacData->jacVTmp1[col + deltaColMod]) - (failedNeg ? 0.0 : jacData->jacVTmp2[col + deltaColMod]);
													jac[row + (col)*jnrows] = (fPert[row] - fPert2[row]) / deltasum;
													jac = NULL;
													fPert = fPert2 = NULL;
												}
											}
										}
									}
								} else goto done2;
								GC_ind = 1;
								GC_num_rows = 0;
								{
									int i;
									for (i = 0; i < GC_size; i++) {
										int col;
										int row;
										if (pair.a == 0) {
											col = b[i];
											row = i;
										} else if (pair.a == 1) {
											if (GC_ind >= GC_new_column + GC_num_rows + 2) {
												GC_new_column = GC_ind;
												col_save = b[GC_ind++];
												GC_num_rows = b[GC_ind++];
											}
											col = col_save;
											row = b[GC_ind++] - 1;
										} else goto done2;
										if (col >= 1) {
											int mat = 0;
											int deltaColMod = 0;
											int jnrows = 0;
											double* jac = NULL;
											double* fPert = NULL;
											double* fPert2 = NULL;
											double deltasum = 0;
											if (row >= nx) {
												row -= nx;
												mat += 1 << 0;
												fPert = failedPos ? comp->outputs : jacData->jacZTmp1 + nx;
												fPert2 = failedNeg ? comp->outputs : jacData->jacZTmp2 + nx;
												jnrows = ny;
											} else {
												fPert = failedPos ? comp->derivatives : jacData->jacZTmp1;
												fPert2 = failedNeg ? comp->derivatives : jacData->jacZTmp2;
												jnrows = nx;
											}
											if (col > nx) {
												col -= nx;
												mat += 1 << 1;
												deltaColMod = nx;
											}
											--col; /*all use of col here is  cindex based so reduce col*/
											jac = mat < 2 ? (mat == 0 ? jacData->jacA : jacData->jacC) : (mat == 2 ? jacData->jacB : jacData->jacD);
											deltasum = (failedPos ? 0.0 : jacData->jacVTmp1[col + deltaColMod]) - (failedNeg ? 0.0 : jacData->jacVTmp2[col + deltaColMod]);
											jac[row + (col)*jnrows] = (fPert[row] - fPert2[row]) / deltasum;
											jac = NULL;
											fPert = fPert2 = NULL;
										}
									}
								}
							}
						}
						/* Group worked: set all elements to up-to-date */
						if (structuredGroup)
							JacInGroupS(jacData->updatedJacobian + 1, b, nx + nu, 1);
						else
							JacInGroup(jacData->updatedJacobian + 1, QJacobianCG_ + groupIx, nx+nu, 1);
					}
					groupIx += 1 + nMembers;
				}
				comp->istruct->mInJacobian=0;
			}
		}
		{
#if (FMI_VERSION >= FMI_3)
		size_t vArrayMod = 0;
		size_t zArrayMod = 0;
#endif
		for (j = 0; j < nv; j++) {
#if (FMI_VERSION >= FMI_3)
			/*Extra code to adjust indexes for array*/
			size_t jk = 0;
			FMIBoolean isArray = FMIFalse;
			FMIValueReference vr = check_arrayVR(v_ref[j], &isArray);
			size_t v_ixx = FMI_INDEX(vr);
			FMICategory v_cat =(FMICategory) FMI_CATEGORY(vr);
			size_t arraySizeV = isArray? arraySizes[FMI_INDEX(v_ref[j])] : 1;
			for(; jk<arraySizeV;++jk){
				size_t jIndx = j+jk+vArrayMod;
				size_t v_ix = v_ixx+jk;
				assert(jIndx<nSeed);

#else
			size_t jIndx = j;
			int v_ix = FMI_INDEX(v_ref[j]);
			FMICategory v_cat =(FMICategory) FMI_CATEGORY(v_ref[j]);
#endif
			if (dv[jIndx] != 0) { /* skip for seed == 0 */
				FMIBoolean vIsState = FMITrue;
				FMIBoolean vIsAlgebraic = FMIFalse;
				switch(v_cat) {
				case fmiState:
					assert(v_ix < nx);
					break;
				case fmiInput:
					assert(v_ix < nu);
					vIsState = FMIFalse;
					break;
				case fmiAlgebraic:
					if (isFmiDAE == FMITrue) {
						assert(v_ix < nx2);
						vIsState = FMIFalse;
						vIsAlgebraic = FMITrue;
						break;
					} /* Fallthrough */
				default:
					util_logger(comp, comp->instanceName, FMIError, loggBadCall,
						"%s: v_ref[%d] refers neither to state nor input", label, i);
					status = FMIError;
					goto done1;
				}

				/* Compute column j */
#if (FMI_VERSION >= FMI_3)
				zArrayMod = 0;
#endif
				for (i = 0; i < nz; i++) {
					/* figure out which of A, B, C and D matrix to fetch from */
#if (FMI_VERSION >= FMI_3)
					size_t ik = 0;
					FMIBoolean isArray = FMIFalse;
					FMIValueReference vr = check_arrayVR(z_ref[i], &isArray);
					size_t z_ixx = FMI_INDEX(vr);
					
					FMICategory z_cat =(FMICategory) FMI_CATEGORY(vr);
					size_t arraySizeZ = isArray? arraySizes[FMI_INDEX(z_ref[i])] : 1;
					for(; ik<arraySizeZ;++ik){
						size_t iIndx = i+ik +zArrayMod;
						size_t z_ix = z_ixx+ik;
						assert(iIndx<nSensitivity);

#else
					FMICategory z_cat = (FMICategory) FMI_CATEGORY(z_ref[i]);
					int z_ix = FMI_INDEX(z_ref[i]);
					size_t iIndx = i;
#endif
					switch (z_cat) {
					case fmiDerivative:
						assert(z_ix < nx);
						if (vIsState == FMITrue) {
							dz[iIndx] += dv[jIndx] * jacData->jacA[z_ix + v_ix * nx];
						}
						else if (vIsAlgebraic == FMITrue) {
							dz[iIndx] += dv[jIndx] * jacData->jacA[z_ix + (v_ix + comp->nxOrig) * nx];
						}
						else {
							dz[iIndx] += dv[jIndx] * jacData->jacB[z_ix + v_ix * nx];
						}
						break;
					case fmiOutput:
						assert(z_ix < ny);
						if (vIsState == FMITrue) {
							dz[iIndx] += dv[jIndx] * jacData->jacC[z_ix + v_ix * ny];
						}
						else if (vIsAlgebraic == FMITrue) {
							dz[iIndx] += dv[jIndx] * jacData->jacC[z_ix + (v_ix + comp->nxOrig) * ny];
						}
						else {
							dz[iIndx] += dv[jIndx] * jacData->jacD[z_ix + v_ix * ny];
						}
						break;
					case fmiResidual:
						assert(z_ix < nx);
						if (vIsState == FMITrue) { 
							dz[iIndx] += dv[jIndx] * jacData->jacA[z_ix + comp->nxOrig + v_ix * nx];
						}
						else if (vIsAlgebraic == FMITrue) {
							dz[iIndx] += dv[jIndx] * jacData->jacA[z_ix + comp->nxOrig + (v_ix + comp->nxOrig) * nx];
						}
						else {
							dz[iIndx] += dv[jIndx] * jacData->jacB[z_ix + comp->nxOrig + v_ix * nx];
						}
						break;
					default:
						util_logger(comp, comp->instanceName, FMIError, loggBadCall,
							"%s: z_ref[%d] refers neither to state derivative nor output", label, i);
						status = FMIError;
						goto done1;
					}
#if (FMI_VERSION >= FMI_3)
					}
				zArrayMod+=arraySizeZ-1;
#endif
				}
			}
#if (FMI_VERSION >= FMI_3)
		}
		vArrayMod+=arraySizeV-1;
#endif
		}
		}

done1:
		SetDymolaJacobianPointers(comp->did, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0);
		HANDLE_STATUS_RETURN(status);
	}

	/*  analytic Jacobian or grouping not available slow fallback solution*/
	{

		double *jacV;
		double *jacVTmp1;
		double *jacVTmp2;
		double *jacZ;
		double *jacZTmp1;
		double *jacZTmp2;
		double h;
		FMIBoolean success1;
		FMIBoolean success2;
#if (FMI_VERSION >= FMI_3)
		size_t nSeed_ = nSeed;
		size_t nSensitivity_ = nSensitivity;
#else
		size_t nSeed_ = nv;
		size_t nSensitivity_= nz;
#endif


		if (directionalABCD && (comp->mStatus != modelInstantiated && comp->mStatus != modelInitializationMode) && !comp->AdvancedDiscreteStates) {
			/* avoid re-allocation on each call */
			int nx, nx2, nu, ny, nw, np, nrel, ncons, dae;

			GetDimensions(&nx, &nx2, &nu, &ny, &nw, &np, &nrel, &ncons, &dae);
			if (DYNAllocateDirHelper(nx+nu, nx+ny, jacData)) {
				util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
				status = FMIError;
				goto done2;
			}
			comp->istruct->mInJacobian = 1;
			for (j = 0; j < (size_t)(nx + nu); ++j) jacData->jacV[j] = 0;

			{
#if (FMI_VERSION >= FMI_3)
				size_t vArrayMod = 0;
				size_t zArrayMod = 0;
#endif
				for (j = 0; j < nv; j++) {
					/* skip for seed == 0 */

#if (FMI_VERSION >= FMI_3)
					/*Extra code to adjust indexes for array*/
					size_t jk = 0;
					FMIBoolean isArray = FMIFalse;
					FMIValueReference vr = check_arrayVR(v_ref[j], &isArray);
					size_t v_ixx = FMI_INDEX(vr);
					FMICategory v_cat = (FMICategory)FMI_CATEGORY(vr);
					size_t arraySizeV = isArray ? arraySizes[FMI_INDEX(v_ref[j])] : 1;
					for (; jk < arraySizeV; ++jk) {
						size_t jIndx = j + jk + vArrayMod;
						size_t v_ix = v_ixx + jk;
						assert(jIndx < nSeed);

#else
					size_t jIndx = j;
					int v_ix = FMI_INDEX(v_ref[j]);
					FMICategory v_cat = (FMICategory)FMI_CATEGORY(v_ref[j]);
#endif
					if (dv[jIndx] != 0) {
						FMIBoolean vIsState = FMITrue;
						/* determine type for v reference*/
						FMICategory v_cat = (FMICategory)FMI_CATEGORY(v_ref[j]);
						switch (v_cat) {
						case fmiState:
							assert(v_ix < nx);
							break;
						case fmiInput:
							assert(v_ix < nu);
							vIsState = FMIFalse;
							break;
						default:
							util_logger(comp, comp->instanceName, FMIError, loggBadCall,
								"%s: v_ref[%d] refers neither to state nor input", label, i);
							status = FMIError;
							goto done1;
						}
						jacData->jacV[(vIsState ? v_ix : v_ix + nx)] = dv[jIndx];
					}
#if (FMI_VERSION >= FMI_3)
					}
				vArrayMod += arraySizeV - 1;
#endif
				}
			}
			{
				int idemand = 2;
				int QiErr = 0;
				comp->enforceRefresh = FMITrue;
				comp->icall = 0;
				SetDymolaDirPointers(comp->did, 1, jacData->jacV, jacData->jacV + nx, jacData->jacZTmp1, jacData->jacZTmp1 + nx);
				QiErr = util_refresh_cache(comp, idemand, NULL, NULL);
				SetDymolaDirPointers(comp->did, 0, 0, 0, 0, 0);
				comp->enforceRefresh = FMIFalse;
				/* Not fully correct, even if we reset inputs and states we don't reset helpvars, so a subsequent call to AcceptedSection (or AdjointSection) may give wrong results */
				/* The derivatives and states are correct though - so we might do something more for that */
				if (QiErr) {
					util_logger(comp, comp->instanceName, FMIError, loggQiErr,
						"%s: dsblock_ failed, QiErr = %d", label, QiErr);
					goto done2;
				}
			}

			{
				/* Compute column j */
#if (FMI_VERSION >= FMI_3)
				size_t zArrayMod = 0;
#endif
				for (i = 0; i < nz; i++) {
					/* figure out which of A, B, C and D matrix to fetch from */
#if (FMI_VERSION >= FMI_3)
					size_t ik = 0;
					FMIBoolean isArray = FMIFalse;
					FMIValueReference vr = check_arrayVR(z_ref[i], &isArray);
					size_t z_ixx = FMI_INDEX(vr);

					FMICategory z_cat = (FMICategory)FMI_CATEGORY(vr);
					size_t arraySizeZ = isArray ? arraySizes[FMI_INDEX(z_ref[i])] : 1;
					for (; ik < arraySizeZ; ++ik) {
						size_t iIndx = i + ik + zArrayMod;
						size_t z_ix = z_ixx + ik;
						assert(iIndx < nSensitivity);

#else
					FMICategory z_cat = (FMICategory)FMI_CATEGORY(z_ref[i]);
					int z_ix = FMI_INDEX(z_ref[i]);
					size_t iIndx = i;
#endif
					switch (z_cat) {
					case fmiDerivative:
						assert(z_ix < nx);
						dz[iIndx] += jacData->jacZTmp1[z_ix];
						break;
					case fmiOutput:
						assert(z_ix < ny);
						dz[iIndx] += jacData->jacZTmp1[z_ix+nx];
						break;
					default:
						util_logger(comp, comp->instanceName, FMIError, loggBadCall,
							"%s: z_ref[%d] refers neither to state derivative nor output", label, i);
						status = FMIError;
						goto done1;
					}
#if (FMI_VERSION >= FMI_3)
					}
				zArrayMod += arraySizeZ - 1;
#endif
				}
			}
		} else {
			/* avoid re-allocation on each call */
			if (DYNAllocateDirHelper(nSeed_, nSensitivity_, jacData)) {
				util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
				status = FMIError;
				goto done2;
			}
			comp->istruct->mInJacobian = 1;
			/* save current v_values for later restoring */
#if (FMI_VERSION >= FMI_3)
			if ((status = fmi3GetFloat64_(c, v_ref, nv, jacData->jacV, nSeed)) != FMIOK ||
				(status = fmi3GetFloat64_(c, z_ref, nz, jacData->jacZ, nSensitivity)) != FMIOK) {
#else
			if ((status = fmiGetReal_(c, v_ref, nv, jacData->jacV)) != FMIOK ||
				(status = fmiGetReal_(c, z_ref, nz, jacData->jacZ)) != FMIOK) {
#endif
				goto done2;
			}
			if (comp->AdvancedDiscreteStates) {
				if ((status = fmiGetFMUstate_(c, &jacData->stateStore)) != FMIOK) {
					goto done2;
				}
				SetDymolaEnforceWhen(comp->did, 1);
				comp->icall = 0;
			}
			jacV = jacData->jacV;
			jacVTmp1 = jacData->jacVTmp1;
			jacVTmp2 = jacData->jacVTmp2;
			jacZ = jacData->jacZ;
			jacZTmp1 = jacData->jacZTmp1;
			jacZTmp2 = jacData->jacZTmp2;

			h = 1e6;
			for (i = 0; i < nSeed_; ++i) {
				if (dv[i] != 0) {
					double temp = (fabs(jacV[i]) + 1.0) * 1.0e-5 / fabs(dv[i]);
					if (temp < h)
						h = temp;
				}
			}

			/* perturb each state/continuous input */
			for (i = 0; i < nSeed_; i++) {
				/* skip for seed == 0 */
				if (dv[i] != 0) {
					jacVTmp1[i] = jacV[i] + h*dv[i];
					jacVTmp2[i] = jacV[i] - h*dv[i];
				} else {
					jacVTmp1[i] = jacVTmp2[i] = jacV[i];

				}
			}
#if (FMI_VERSION >= FMI_3)
			if ((status = fmi3SetFloat64_(c, v_ref, nv, jacVTmp1, nSeed)) != FMIOK) {
#else
			if ((status = fmiSetReal_(c, v_ref, nv, jacVTmp1)) != FMIOK) {
#endif
				goto done2;
			}
			comp->enforceRefresh = FMITrue;
			comp->icall = 0;
#if (FMI_VERSION >= FMI_3)
			status = fmi3GetFloat64_(c, z_ref, nz, jacZTmp1, nSensitivity);
#else
			status = fmiGetReal_(c, z_ref, nz, jacZTmp1);
#endif
			comp->enforceRefresh = FMIFalse;
			if (comp->AdvancedDiscreteStates) {
				if ((status = fmiSetFMUstate_(c, jacData->stateStore, FMITrue)) != FMIOK) {
					goto done2;
				}
				SetDymolaEnforceWhen(comp->did, 1);
			}
			success1 = status == FMIOK;
#if (FMI_VERSION >= FMI_3)
			if ((status = fmi3SetFloat64_(c, v_ref, nv, jacVTmp2, nSeed)) != FMIOK) {
#else
			if ((status = fmiSetReal_(c, v_ref, nv, jacVTmp2)) != FMIOK) {
#endif
				goto done2;
			}
			comp->enforceRefresh = FMITrue;
			comp->icall = 0;
#if (FMI_VERSION >= FMI_3)
			status = fmi3GetFloat64_(c, z_ref, nz, jacZTmp2, nSensitivity);
#else
			status = fmiGetReal_(c, z_ref, nz, jacZTmp2);
#endif
			comp->enforceRefresh = FMIFalse;
			success2 = status == FMIOK;

			if (success1 && success2) {
				/*Central Difference*/
				for (i = 0; i < nSensitivity_; i++) {
					dz[i] = (FMIReal)(jacZTmp1[i] - jacZTmp2[i]) / (2.0 * h);
				}
			} else if (success1) {
				/*Forwad Difference*/
				for (i = 0; i < nSensitivity_; i++) {
					dz[i] = (FMIReal)(jacZTmp1[i] - jacZ[i]) / h;
				}
			} else if (success2) {
				/*Backward Difference*/
				for (i = 0; i < nSensitivity_; i++) {
					dz[i] = (FMIReal)(jacZ[i] - jacZTmp2[i]) / h;
				}
			} else {
				goto done2;
			}
			/* Restore */
			if (comp->AdvancedDiscreteStates) {
				if ((status = fmiSetFMUstate_(c, jacData->stateStore, FMITrue)) != FMIOK) {
					goto done2;
				}
				SetDymolaEnforceWhen(comp->did, 0);
#if (FMI_VERSION >= FMI_3)
			} else if ((status = fmi3SetFloat64_(c, v_ref, nv, jacV, nSeed)) != FMIOK) {
#else
			} else if ((status = fmiSetReal_(c, v_ref, nv, jacV)) != FMIOK) {
#endif
				goto done2;
			}
		}
done2:
		comp->istruct->mInJacobian=0;
		HANDLE_STATUS_RETURN(status);
	}
}

#if (FMI_VERSION >= FMI_3)
DYMOLA_STATIC FMIStatus fmi3GetAdjointDerivative_(fmi3Instance instance,
                                                const fmi3ValueReference unknowns[],
                                                size_t nUnknowns,
                                                const fmi3ValueReference knowns[],
                                                size_t nKnowns,
                                                const fmi3Float64 seed[],
                                                size_t nSeed,
                                                fmi3Float64 sensitivity[],
                                                size_t nSensitivity){
	static FMIString label = "fmi3GetAdjointDerivative";
	size_t i;
	size_t j;
	Component* comp = (Component*) instance;
	FMIStatus status = FMIOK;
	int analyticABCD, analyticAdjoint;

	JacobianData* jacData = &comp->jacData;
	extern int QJacobianDefined_;
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s", label);

	if(nSeed > nUnknowns){
		static const char* get ="";
		ERROR_RETURN_CHECK(checkSizes(comp, unknowns, nUnknowns, nSeed, label, get));
	}
	if(nSensitivity > nKnowns){
		static const char* get ="";
		ERROR_RETURN_CHECK(checkSizes(comp, knowns, nKnowns, nSensitivity, label, get));
	}

	for (i = 0; i < nSensitivity; i++) {
		sensitivity[i] = 0;
	}
	analyticABCD = (QJacobianDefined_ & 3) == 3;
	analyticAdjoint = (QJacobianDefined_ & 4) == 4;
	/*  analytic Jacobian available ? */
	/* Either ABCD or Adjoint */
	if (!analyticABCD && !analyticAdjoint) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, 
			"%s: Currently only supported with analytical jacobians this can be set during FMU export", label);
		return util_error(comp);
	}

	if(comp->mStatus == modelInstantiated ){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, 
			"%s: Current mode is Instantiated mode, currently only supported during continuous time mode", label);
		return util_error(comp);
	}

	if(comp->mStatus == modelInitializationMode){
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, 
			"%s: Current mode is Initialization mode, currently only supported during continuous time mode", label);
		return util_error(comp);
	}
	if (analyticAdjoint && (comp->mStatus != modelInstantiated &&
		comp->mStatus != modelInitializationMode) && !comp->AdvancedDiscreteStates) {
		/* Reverse adjoint */
		/* Note: No actual caching */
		int nx, nx2, nu, ny, nw, np, nsp, nrel2, nrel, ncons, dae, nd, nxp, nwp, nwc, nhelp2, naux;
		GetDimensions5(&nx, &nx2, &nu, &ny, &nw, &np, &nsp, &nrel2, &nrel, &ncons, &dae, &nd, &nxp, &nwp, &nwc, &nhelp2, &naux);

		FMIBoolean isFmiDAE = GetAdditionalFlags(40);
		if (isFmiDAE) {
			comp->nxOrig = nx;
			nx = nx + nx2;
		}

		if (DYNAllocateDirHelper(nx + nw + nhelp2 + nu + ny + nx + np + naux+ 1, 0, jacData)) {
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
			status = FMIError;
			goto done1;
		}
		/* Set up seeds */
		{
			int i;
			for (i = 0; i < nx + nw + nhelp2 + nu + ny + nx + np+ naux; ++i) jacData->jacV[i] = 0;
		}
		{
			size_t seedArrayMod = 0;
			for (j = 0; j < nSeed; j++) {
				/* skip for seed == 0 */
				FMIBoolean isArray = FMIFalse;
				FMIValueReference vr = check_arrayVR(unknowns[j], &isArray);
				size_t arraySizeSeed = isArray ? arraySizes[FMI_INDEX(unknowns[j])] : 1;
				size_t v_ixx = FMI_INDEX(vr);
				FMICategory v_cat = (FMICategory)FMI_CATEGORY(vr);
				size_t jk = 0;
				for (; jk < arraySizeSeed; ++jk) {
					size_t jIndx = j + jk + seedArrayMod;
					size_t v_ix = v_ixx + jk;
					assert(jIndx < nSeed);
					if (seed[jIndx] != 0) {
						switch (v_cat) {
						case fmiDerivative:
							assert(v_ix < nx);
							jacData->jacV[v_ix] = seed[jIndx];
							break;
						case fmiOutput:
							assert(v_ix < ny);
							jacData->jacV[nx + nw + nhelp2 + nu + v_ix] = seed[jIndx];
							break;
						case fmiResidual:
							assert(v_ix < nx2);
							jacData->jacV[comp->nxOrig + v_ix] = seed[jIndx];
							break;
						default:
							util_logger(comp, comp->instanceName, FMIError, loggBadCall,
								"%s: v_ref[%d] refers neither to state derivative nor output", label, i);
							status = FMIError;
							goto done1;
						}

					}
				}
				seedArrayMod += arraySizeSeed - 1;
			}
		}
		/* Actual computation. Note: Adjoint pointers are only set during this part */
		SetDymolaAdjointPointers(comp->did, jacData->jacV /*QFR*/, jacData->jacV + nx /*QWR*/, jacData->jacV + nx + nw /* QHelp*/, jacData->jacV + nx + nw + nhelp2 /* QUR*/,
			jacData->jacV + nx + nw + nhelp2 + nu /* QYR*/,
			jacData->jacV + nx + nw + nhelp2 + nu + ny /* QXR*/,
			jacData->jacV + nx + nw + nhelp2 + nu + ny + nx /* QPR*/,
			jacData->jacV + nx + nw + nhelp2 + nu + ny + nx+np /* QAux*/
			);
		{
			int res;
			/* Should not be needed: */
			comp->enforceRefresh = FMITrue; 
			comp->icall = 0;
			/* But safe while waiting for other changes */
			res = util_refresh_cache(comp, iDemandDerivative, label, NULL);

			SetDymolaAdjointPointers(comp->did, 0 /*QFR*/, 0 /*QWR*/, 0 /* QHelp*/, 0 /* QU*/, 0 /*QY*/,
				0 /* QXR*/,
				0 /* QPR*/,
				0 /* QAux*/);
			if (res) {
				status = FMIError;
				goto doneAdj1;
			}
		}
		{
			/* Extract values */
			size_t sensitivityArrayMod = 0;
			for (i = 0; i < nSensitivity; i++) {
				/* figure out which of A, B, C and D matrix to fetch from */
				size_t ik = 0;
				FMIBoolean isArray = FMIFalse;
				FMIValueReference vr = check_arrayVR(knowns[i], &isArray);
				size_t arraySizeSensitivity = isArray ? arraySizes[FMI_INDEX(knowns[i])] : 1;
				size_t z_ixx = FMI_INDEX(vr);
				FMICategory z_cat = (FMICategory)FMI_CATEGORY(vr);
				for (; ik < arraySizeSensitivity; ++ik) {
					size_t iIndx = i + ik + sensitivityArrayMod;
					size_t z_ix = z_ixx + ik;
					assert(iIndx < nSensitivity);
					switch (z_cat) {
					case fmiState:
						assert(z_ix < nx);
						sensitivity[iIndx] += jacData->jacV[nx + nw + nhelp2 + nu + ny + z_ix];
						break;
					case fmiInput:
						assert(z_ix < nu);
						sensitivity[iIndx] += jacData->jacV[nx + nw + nhelp2 + z_ix];
						break;
					case fmiParameter:
						assert(z_ix < np);
						sensitivity[iIndx] += jacData->jacV[nx + nw + nhelp2 + nu + ny + nx + z_ix];
						break;
					case fmiAlgebraic:
						assert(z_ix < nx2);
						sensitivity[iIndx] += jacData->jacV[nx + nw + nhelp2 + nu + ny + comp->nxOrig + z_ix];
						break;
					case fmiSystemVar:
						if (z_ix == 0xffffff) {
							/* Time */
							sensitivity[iIndx] += jacData->jacV[nx + nw + nhelp2-1];
							break;
						}
					default:
						util_logger(comp, comp->instanceName, FMIError, loggBadCall,
							"%s: z_ref[%d] refers neither to state, parameter, nor input", label, i);
						status = FMIError;
						goto done1;
					}
				}
				sensitivityArrayMod += arraySizeSensitivity - 1;
			}
		}
	doneAdj1:
		HANDLE_STATUS_RETURN(status);
	} else if (analyticABCD && (comp->mStatus != modelInstantiated &&
		comp->mStatus != modelInitializationMode) && !comp->AdvancedDiscreteStates ) {
			/*Analytical Jacobians are only valid after intialization*/
		int nx, nx2, nu, ny, nw, np, nrel, ncons, dae;
		int nA, nB, nC, nD;
		
		GetDimensions(&nx, &nx2, &nu, &ny, &nw, &np, &nrel, &ncons, &dae);

		FMIBoolean isFmiDAE = analyticABCD && GetAdditionalFlags(40);
		if (isFmiDAE) {
			comp->nxOrig = nx;
			nx = nx + nx2;
		}

		nA = nx * nx;
		nB = nx * nu;
		nC = ny * nx;
		nD = ny * nu;

		/* avoid re-allocation on each call */
		if (jacData->nJacA < nA || jacData->jacA == NULL) {
			FREE_MEMORY(jacData->jacA);
			jacData->jacA = (double*) ALLOCATE_MEMORY(nA+1, sizeof(double));
		}
		if (jacData->nJacB < nB || jacData->jacB == NULL) {
			FREE_MEMORY(jacData->jacB);
			jacData->jacB = (double*) ALLOCATE_MEMORY(nB+1, sizeof(double));
		}
		if (jacData->nJacC < nC || jacData->jacC == NULL) {
			FREE_MEMORY(jacData->jacC);
			jacData->jacC = (double*) ALLOCATE_MEMORY(nC+1, sizeof(double));
		}
		if (jacData->nJacD < nD || jacData->jacD == NULL) {
			FREE_MEMORY(jacData->jacD);
			jacData->jacD = (double*) ALLOCATE_MEMORY(nD+1, sizeof(double));
		}

		if (jacData->jacA == NULL || jacData->jacB == NULL || jacData->jacC== NULL || jacData->jacD == NULL) {
			util_logger(comp, comp->instanceName, FMIError, loggMemErr, "%s: memory allocation failed", label);
			status = FMIError;
			goto done1;
		}

		jacData->nJacA = nA;
		jacData->nJacB = nB;
		jacData->nJacC = nC;
		jacData->nJacD = nD;		


		if(comp->icall < iDemandDerivative && comp->mStatus != modelInitializationMode || comp->recalJacobian){
			SetDymolaJacobianPointers(comp->did, jacData->jacA, jacData->jacB, jacData->jacC, jacData->jacD, nx, nu, ny, 0, 0,0,0);
			if (util_refresh_cache(comp, iDemandDerivative, label, NULL)) {
				status = FMIError;
				goto done1;
			}
			comp->recalJacobian = 0;
		}

		{
			size_t seedArrayMod = 0;
			size_t sensitivityArrayMod = 0;
			for (j = 0; j < nSeed; j++) {
				/* skip for seed == 0 */
				FMIBoolean isArray = FMIFalse;
				FMIValueReference vr = check_arrayVR(unknowns[j], &isArray);
				size_t arraySizeSeed = isArray? arraySizes[FMI_INDEX(unknowns[j])] : 1;
				size_t v_ixx = FMI_INDEX(vr);
				FMICategory v_cat =(FMICategory) FMI_CATEGORY(vr);
				size_t jk = 0;
				for(; jk<arraySizeSeed;++jk){
					size_t jIndx = j+jk+seedArrayMod;
					size_t v_ix = v_ixx+jk;
					assert(jIndx<nSeed);
					if (seed[jIndx] != 0) {
						FMIBoolean isDerivative = FMITrue;
						FMIBoolean isResidual = FMIFalse;
						switch(v_cat) {
						case fmiDerivative:
							assert(v_ix < nx);
							break;
						case fmiOutput:
							assert(v_ix < ny);
							isDerivative = FMIFalse;
							break;
						case fmiResidual:
							if (isFmiDAE == FMITrue) {
								assert(v_ix < nx2);
								isDerivative = FMIFalse;
								isResidual = FMITrue;
								break;
							}
						default:
							util_logger(comp, comp->instanceName, FMIError, loggBadCall,
								"%s: v_ref[%d] refers neither to state nor input", label, i);
							status = FMIError;
							goto done1;
						}

						/* Compute column j */
						for (i = 0; i < nSensitivity; i++) {
							/* figure out which of A, B, C and D matrix to fetch from */
							size_t ik = 0;
							FMIBoolean isArray = FMIFalse;
							FMIValueReference vr = check_arrayVR(knowns[i], &isArray);
							size_t arraySizeSensitivity = isArray? arraySizes[FMI_INDEX(knowns[i])] : 1;
							size_t z_ixx = FMI_INDEX(vr);
							FMICategory z_cat =(FMICategory) FMI_CATEGORY(vr);
							for(; ik<arraySizeSensitivity;++ik){
								size_t iIndx = i+ik +sensitivityArrayMod;
								size_t z_ix = z_ixx+ik;
								assert(iIndx<nSensitivity);
								switch (z_cat) {
								case fmiState:
									assert(z_ix < nx);
									if (isDerivative == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacA[z_ix * nx + v_ix ];
									} else if (isResidual == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacA[z_ix * nx + v_ix + comp->nxOrig];
									} else {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacC[z_ix * ny + v_ix ];
									}
									break;
								case fmiInput:
									assert(z_ix < nu);
									if (isDerivative == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacB[z_ix * nx + v_ix];
									} else if (isResidual == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacB[z_ix * nx + v_ix + comp->nxOrig];
									} else {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacD[z_ix * ny + v_ix];
									}
									break;
								case fmiAlgebraic:
									assert(z_ix < nx2);
									if (isDerivative == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacA[(z_ix + comp->nxOrig) * nx + v_ix];
									} else if (isResidual == FMITrue) {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacA[(z_ix + comp->nxOrig) * nx + v_ix + comp->nxOrig];
									} else {
										sensitivity[iIndx] += seed[jIndx] * jacData->jacC[(z_ix + comp->nxOrig) * ny + v_ix];
									}
									break;
								default:
									util_logger(comp, comp->instanceName, FMIError, loggBadCall,
										"%s: z_ref[%d] refers neither to state derivative nor output", label, i);
									status = FMIError;
									goto done1;
								}
							}
							sensitivityArrayMod+=arraySizeSensitivity-1;
						}
					}
				}
				seedArrayMod+=arraySizeSeed-1;
			}
		}
done1:
		SetDymolaJacobianPointers(comp->did, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0);
		HANDLE_STATUS_RETURN(status);

	}
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3EnterConfigurationMode_(fmi3Instance instance) {
	static FMIString label = "fmi3EnterConfigurationMode";
	Component* comp = (Component*)instance;
	if (comp->mStatus == modelInstantiated) {
		comp->mStatus = modelConfigurationMode;
		return FMIOK;
	}
	util_logger(comp, comp->instanceName, FMIError, loggBadCall,
		"%s: is only supported before fmi3EnterInitializationMode is called.", label);
	return util_error(comp);
}


extern void SetRealDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims);
extern void SetIntegerDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims);
extern void SetStringDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims);

DYMOLA_STATIC FMIStatus fmi3ExitConfigurationMode_(fmi3Instance instance) {
	static FMIString label = "fmi3ExitConfigurationMode";
	Component* comp = (Component*)instance;
	if (comp->mStatus == modelConfigurationMode) {
		size_t i, nbrReal = comp->dynArraySize.nbrReal, nbrInt = comp->dynArraySize.nbrInt, nbrStr = comp->dynArraySize.nbrStr;
		for (i = 0; i < nbrReal; ++i) {
			int nDims = 1; //Change later
			int dim[1] = { (int)comp->dynArraySize.dim[i] };
			SetRealDymArraySize(comp->did, (int)i, nDims, dim);
		}
		for (i = 0; i < nbrInt; ++i) {
			int nDims = 1; //Change later
			int dim[1] = { (int)comp->dynArraySize.dim[i + nbrReal] };
			SetIntegerDymArraySize(comp->did, (int)i, nDims, dim);
		}
		for (i = 0; i < nbrStr; ++i) {
			int nDims = 1; //Change later
			int dim[1] = { (int)comp->dynArraySize.dim[i + nbrReal + nbrInt] };
			SetStringDymArraySize(comp->did, (int)i, nDims, dim);
		}
		comp->mStatus = modelInstantiated;
		return FMIOK;
	}
	util_logger(comp, comp->instanceName, FMIError, loggBadCall,
		"%s: is only supported during ConfigurationMode.", label);
	return util_error(comp);
}



DYMOLA_STATIC FMIStatus fmi3GetIntervalDecimal_(fmi3Instance instance,
												const fmi3ValueReference valueReferences[],
												size_t nValueReferences,
												fmi3Float64 intervals[],
												fmi3IntervalQualifier qualifiers[]) {
	unSuportedFunctionCall(instance, "fmi3GetIntervalDecimal");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3GetIntervalFraction_(fmi3Instance instance,
												 const fmi3ValueReference valueReferences[],
												 size_t nValueReferences,
												 fmi3UInt64 intervalCounters[],
												 fmi3UInt64 resolutions[],
												 fmi3IntervalQualifier qualifiers[]) {
	unSuportedFunctionCall(instance, "fmi3GetIntervalDecimal");
	return util_error((Component*)instance);
 }


DYMOLA_STATIC FMIStatus fmi3GetShiftDecimal_(fmi3Instance instance,
											 const fmi3ValueReference valueReferences[],
											 size_t nValueReferences,
											 fmi3Float64 shifts[]) {
	unSuportedFunctionCall(instance, "fmi3GetShiftDecimal");
	return util_error((Component*)instance);
}


DYMOLA_STATIC FMIStatus fmi3GetShiftFraction_(fmi3Instance instance,
											  const fmi3ValueReference valueReferences[],
											  size_t nValueReferences,
											  fmi3UInt64 shiftCounters[],
											  fmi3UInt64 resolutions[]) {
	unSuportedFunctionCall(instance, "fmi3GetShiftFraction");
	return util_error((Component*)instance);
}


DYMOLA_STATIC FMIStatus fmi3SetIntervalDecimal_(fmi3Instance instance,
												const fmi3ValueReference valueReferences[],
												size_t nValueReferences,
												const fmi3Float64 intervals[]) {
	unSuportedFunctionCall(instance, "fmi3SetIntervalDecimal");
	return util_error((Component*)instance);
}

DYMOLA_STATIC FMIStatus fmi3SetIntervalFraction_(fmi3Instance instance,
												 const fmi3ValueReference valueReferences[],
												 size_t nValueReferences,
												 const fmi3UInt64 intervalCounters[],
												 const fmi3UInt64 resolutions[]) {
	unSuportedFunctionCall(instance, "fmi3SetIntervalFraction");
	return util_error((Component*)instance);
}

fmi3Status fmi3SetShiftDecimal_(fmi3Instance instance,
	const fmi3ValueReference valueReferences[],
	size_t nValueReferences,
	const fmi3Float64 shifts[]) {
	unSuportedFunctionCall(instance, "fmi3SetShiftDecimal");
	return util_error((Component*)instance);
}


fmi3Status fmi3SetShiftFraction_(fmi3Instance instance,
	const fmi3ValueReference valueReferences[],
	size_t nValueReferences,
	const fmi3UInt64 shiftCounters[],
	const fmi3UInt64 resolutions[]) {
	unSuportedFunctionCall(instance, "fmi3SetShiftFraction");
	return util_error((Component*)instance);
}


DYMOLA_STATIC FMIStatus fmi3EvaluateDiscreteStates_(fmi3Instance instance){
	unSuportedFunctionCall(instance, "fmi3EvaluateDiscreteStates");
	return util_error((Component*)instance);
}

#endif

/* Wrapper functions for merging CS and ME functions with fmi 2 RC1 */
#if (FMI_VERSION == FMI_2)
DYMOLA_STATIC FMIComponent fmiInstantiate_(FMIString instanceName, FMIType fmuType, FMIString fmuGUID, FMIString fmuResourceLocation, const FMICallbackFunctions* functions, FMIBoolean visible, FMIBoolean loggingOn){

	Component* comp;
	if(fmuType==FMICoSimulation){
		comp = (Component*) fmiInstantiateSlave_(instanceName, fmuGUID, fmuResourceLocation, functions, visible, loggingOn);
		if(comp)
			comp->isCoSim = FMITrue;
	}else if(fmuType==FMIModelExchange){
		comp = (Component*) fmiInstantiateModel_(instanceName, fmuGUID, fmuResourceLocation, functions, visible, loggingOn);
		if(comp)
			comp->isCoSim = FMIFalse;
	}else{
		return NULL;
	}
	return comp;
}
#endif

DYMOLA_STATIC void fmiFreeInstance_(FMIComponent c)
{
	Component* comp = (Component*) c;
	if(comp){
		if(comp->isCoSim)
			fmiFreeSlaveInstance_(c);
		else
			fmiFreeModelInstance_(c);
	}
}

DYMOLA_STATIC FMIStatus fmiSetupExperiment_(FMIComponent c,
											FMIBoolean relativeToleranceDefined,
											FMIReal relativeTolerance,
											FMIReal tStart,
											FMIBoolean tStopDefined,
											FMIReal tStop)
{
	/* TODO fix warning of unsuported features for CS/ME now instead of at initialization */

#if (FMI_VERSION > FMI_2)
	static FMIString label = "fmi3EnterInitializationMode";
#else
	static FMIString label = "fmi2SetupExperiment";
#endif
	Component* comp = (Component*) c;
	comp->tStart = tStart;
	comp->StopTimeDefined = tStopDefined;
	comp->tStop = tStop;
	if(relativeToleranceDefined && !comp->isCoSim ){
#if (FMI_VERSION > FMI_2)
		util_logger(comp, comp->instanceName, FMIWarning, loggBadCall, "%s: tolerance control not supported for fmi3ModelExchange, setting toleranceDefined to fmi3False",label);
#else
		util_logger(comp, comp->instanceName, FMIWarning, loggUndef, "%s: tolerance control not supported for fmuType fmi2ModelExchange, setting toleranceDefined to fmi2False",label);
#endif
		comp->relativeToleranceDefined = FMIFalse;
		comp->relativeTolerance =0;
	}else{
		comp->relativeToleranceDefined = relativeToleranceDefined;
		comp->relativeTolerance = relativeTolerance;
	}
	comp->time = tStart;
	comp->icall = iDemandStart;
#if (FMI_VERSION > FMI_2)
	util_logger(comp, comp->instanceName, FMIOK, loggFuncCall, "%s: startTime is set to %g",label, tStart);
#endif
	return FMIOK;
}
DYMOLA_STATIC int fmiUser_Initialize();

#if FMI_VERSION >= FMI_3
DYMOLA_STATIC FMIStatus fmi3EnterInitializationMode_(FMIComponent c, fmi3Boolean toleranceDefined, fmi3Float64 tolerance,
													 fmi3Float64 startTime, fmi3Boolean stopTimeDefined, fmi3Float64 stopTime)
#else
DYMOLA_STATIC FMIStatus fmiEnterInitializationMode_(FMIComponent c)
#endif
{
	Component* comp = (Component*) c;
	if (fmiUser_Initialize())
		return util_error(comp);
	if (comp->mStatus != modelInstantiated) {
		util_logger(comp, comp->instanceName, FMIError, loggBadCall, "model cannot be initialized in current state(%d)", comp->mStatus);
		return util_error(comp);
	}
#if (FMI_VERSION >= FMI_3)
	ERROR_RETURN_CHECK(fmiSetupExperiment_(c,toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime))
#endif
	comp->fmi2ComputeInit = FMITrue;
	comp->mStatus = modelInitializationMode;
	return FMIOK;
}
DYMOLA_STATIC FMIStatus fmiExitInitializationMode_(FMIComponent c)
{
	Component* comp = (Component*) c;
	if(comp->isCoSim){
		return fmiExitSlaveInitializationMode_(comp);
	}else if(!comp->isCoSim){
		return fmiExitModelInitializationMode_(comp);
	}
	return util_error(comp);
}

DYMOLA_STATIC FMIStatus fmiTerminate_(FMIComponent c)
{
	Component* comp = (Component*) c;
	if(comp->isCoSim){
		return fmiTerminateSlave_(comp);
	}else if(!comp->isCoSim){
		return fmiTerminateModel_(comp);
	}
	return util_error(comp);
}

DYMOLA_STATIC FMIStatus fmiReset_(FMIComponent c)
{
	return fmiResetSlave_(c);
}

/* ----------------- local function definitions ----------------- */

/* -------------------------------------------------------------- */
static FMIStatus allocateFMUState(FMIFMUstate* FMUstate, Component* source)
{
#define staticListSize 128
	Component* target;
	void* freeListStatic[staticListSize];
	void** freeList = 0;
	int freeIndex = 0, freeListDynamic = 0;
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
	size_t i = 0, nbrStaticAllocs = 20; /*Acutal nbr static allocs is 16*/

	if(source->nSPar>100){
		freeList = (void**) ALLOCATE_MEMORY(source->nSPar+nbrStaticAllocs,sizeof(void*));
		if(NULL == freeList){
			return util_error(source);
		}
		freeListDynamic = 1;
	}else{
		freeList = freeListStatic;
	}

	ALLOC_AND_CHECK2(*FMUstate, 1, Component);
	target = (Component*)*FMUstate;
	target->iData = NULL;
	/* set to true when dsmodel data also allocated (which is possibly obsolete),
	   outside of this function */
	target->allocDone = 0;

	/* get state vectors, derivatives, etc */
	target->nStates = source->nStates;
	target->nPar = source->nPar;
	target->nIn = source->nIn;
	target->nOut = source->nOut;
	target->nAux = source->nAux;
	target->nCross = source->nCross;
	target->nSPar = source->nSPar;
	target->nDStates = source->nDStates;
	target->nPrevious = source->nPrevious;

	target->DAEMode = source->DAEMode;
	target->nxOrig = source->nxOrig;

	target->useInputSmoother = source->useInputSmoother;
	target->usePredictorCompensation = source->usePredictorCompensation;

	ALLOC_AND_CHECK2(target->states, source->nStates + 1, FMIReal);
	ALLOC_AND_CHECK2(target->oldStates, source->nStates + 1, FMIReal);
	ALLOC_AND_CHECK2(target->derivatives, source->nStates + 1, FMIReal);
	ALLOC_AND_CHECK2(target->parameters, source->nPar + 1, FMIReal);
	ALLOC_AND_CHECK2(target->inputs, source->nIn + 1, FMIReal);
	ALLOC_AND_CHECK2(target->outputs, source->nOut + 1, FMIReal);
	ALLOC_AND_CHECK2(target->auxiliary, source->nAux + 1, FMIReal);
	ALLOC_AND_CHECK2(target->crossingFunctions, source->nCross + 1, FMIReal);
	ALLOC_AND_CHECK2(target->statesNominal, source->nStates + 1, FMIReal);
	ALLOC_AND_CHECK2(target->sParameters, source->nSPar + 1, FMIChar*);
	ALLOC_AND_CHECK2(target->dStates, source->nDStates + 1, FMIReal);
	ALLOC_AND_CHECK2(target->previousVars, source->nPrevious + 1, FMIReal);
	for (i = 0; i < target->nSPar; ++i) {
		ALLOC_AND_CHECK2(target->sParameters[i], maxStringSize + 1, FMIChar);
	}
	target->sParameters[target->nSPar] = NULL;
	if (target->DAEMode) {
		ALLOC_AND_CHECK2(target->DAEDerivatives, source->nStates + 1, FMIReal);
	}

	ALLOC_AND_CHECK(target->dstruct, 1, sizeof(struct BasicDDymosimStruct));
	ALLOC_AND_CHECK(target->istruct, 1, sizeof(struct BasicIDymosimStruct));

	if (dyn_allowMultipleInstances) {

		ALLOC_AND_CHECK(target->did, 1, dyn_allowMultipleInstances);
		DYNInitializeDid(target->did);
	}
	else {
		/*we still need to copy did */
		ALLOC_AND_CHECK(target->did, 1, dyn_instanceDataSize);
		DYNInitializeDid(target->did);
	}

	{
		delayStruct *del = NULL;
		size_t nDel;
		extern int Buffersize;
		getDelayStruct(target->did, &del, &nDel);
		allocDelayBuffers(del, nDel, Buffersize);
	}
	

#if (FMI_VERSION >= FMI_2) && !(defined(NO_FILE) || defined(RTTWINCAT))
	target->serializedDelayData = NULL;
#endif
	if(freeListDynamic){
		FREE_MEMORY(freeList);
		freeList=0;
		freeListDynamic = 0;
	}

	return FMIOK;

mem_fail:
	{
		int i;
		util_logger(source, source->instanceName, FMIError, loggMemErr, "memory allocation failed");
		for (i = freeIndex - 1; i >= 0; i--) {
			FREE_MEMORY(freeList[i]);
			freeList[i]=0;
		}
		if(freeListDynamic){
			FREE_MEMORY(freeList);
			freeList=0;
			freeListDynamic = 0;
		}
		*FMUstate = NULL;
		return util_error(source);
	}
}
#undef staticListSize

/* -------------------------------------------------------------- */
#if !(defined(NO_FILE) || defined(RTTWINCAT))
static tpl_node* createTplMap(Component* comp, tpl_bin *tplDidBin)
{
	tpl_node* tn;
	delayStruct *del = NULL;
	size_t delayBufferDataSize, nDel;
	/* Should we include nRequiredIntermediateInputs? */
	/* skipping const to avoid compiler warning when calling tpl_pack */
	char* TPL_STRUCTURE_STRING =
		"if"                        /* relativeToleranceDefined, relativeTolerance */
        "i"                         /* loggingOn */     
		"f#f#f#f#f#f#f#f#f#f#s#"    /* states, derivates etc */
		"fiiiiiiiiiiiiUfiiiUii"     /* time, icall, mStatus etc */
		"S(ffffff)S(iiiiiiiiiiiii)" /* dstruct, istruct */
		"B"                         /* did */
		"f#"                        /* serializedDelayData */
		"iif"                       /* steadyStateReached, maxRunTimeReached, oneSimulationTotalTimerStart */

#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
	/* TODO: Need to deal with Sundials data structures to handle co-simulation case */
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */
		;

	/* setup TPL hooks during first use */
	if (globalComponent != NULL) {
		static int tplhooks_setup = 0;
		if (!tplhooks_setup) {
			DYMOLA_STATIC tpl_hook_t tpl_hook; /*if not source fmu tpl_hook is in another compilation unit*/
			tpl_hook.tpl_malloc = tplMalloc;
			tpl_hook.tpl_free = tplFree;
			tplhooks_setup = 1;
		}
	}

    /* setting addr and sz is only significant for serialization; for deserialziation
       tpl sets the values */
	tplDidBin->addr = comp->did;
	tplDidBin->sz = (uint32_t) dyn_allowMultipleInstances;

	getDelayStruct(comp->did,&del,&nDel);

	delayBufferDataSize = nDel*del[0].nx*2;
	/* add one to handle empty arrays (size 0 not allowed in tpl_map) */
	tn = tpl_map(TPL_STRUCTURE_STRING,
        &(comp->relativeToleranceDefined),
		&(comp->relativeTolerance),
 		&(comp->loggingOn),
		comp->states, comp->nStates + 1,
		comp->derivatives, comp->nStates + 1,
		comp->parameters, comp->nPar + 1,
		comp->inputs, comp->nIn + 1,
		comp->outputs, comp->nOut + 1,
		comp->auxiliary, comp->nAux + 1,
		comp->crossingFunctions, comp->nCross + 1,
		comp->statesNominal, comp->nStates + 1,
		comp->dStates, comp->nDStates + 1,
		comp->previousVars, comp->nPrevious + 1,
		comp->sParameters, comp->nSPar + 1,
		&(comp->time),
		&(comp->icall),
		&(comp->mStatus),
		&(comp->terminationByModel),
		&(comp->storeResult),
		&(comp->valWasSet),
		&(comp->reinitCvode),
		&(comp->eventIterRequired),
		&(comp->recalJacobian),
		&(comp->enforceRefresh),
		&(comp->hycosimInputEvent),
		&(comp->firstEventCall),
		&(comp->eventIterationOnGoing),
        &(comp->inlineStepCounter),
        &(comp->nextResultSampleTime),
		&(comp->AdvancedDiscreteStates),
		&(comp->fmi2ComputeInit),
		&(comp->DAEMode),
		&(comp->nxOrig),
		&(comp->useInputSmoother),
		&(comp->usePredictorCompensation),
		comp->dstruct,
		comp->istruct,
		tplDidBin,
		comp->serializedDelayData,delayBufferDataSize,
		&(comp->steadyStateReached),
		&(comp->maxRunTimeReached),
		&(comp->oneSimulationTotalTimerStart)
#ifndef ONLY_INCLUDE_INLINE_INTEGRATION
		/* TODO: Need to deal with Sundials data structures to handle co-simulation case */
#endif /* ONLY_INCLUDE_INLINE_INTEGRATION */              
	);

	if (tn == NULL) {
		util_logger(comp, comp->instanceName, FMIError, loggUndef, "createTplMap: tpl_map returned NULL");
	}

	return tn;
}

/* -------------------------------------------------------------- */
static void *tplMalloc(size_t size)
{
	/* in case globalComponent no longer valid, use ordinary malloc as fallback */
	if (globalComponent == NULL) {
		return malloc(size);
	}
	return ALLOCATE_MEMORY(1, size);
}

/* -------------------------------------------------------------- */
static void tplFree(void* ptr)
{
	/* in case globalComponent no longer valid, use ordinary free as fallback */
	if (globalComponent == NULL) {
		free(ptr);
        return;
	}
	FREE_MEMORY(ptr);
}
#endif
#endif /* FMI_2 */

/* -------------------------------------------------------------- */
static int getLargestIdemand(const FMIValueReference vr[], size_t nvr)
{
	int idemand = iDemandStart;
	size_t i;

	/* find biggest idemand among requested values */
	for (i = 0; i < nvr; i++) {
		const FMIValueReference r = vr[i];
		int id = FMI_IDEMAND(r);
		if (id > idemand) {
			idemand = id;
		}
	}
	return idemand;
}

#if (FMI_VERSION >= FMI_2)
/* -------------------------------------------------------------- */
static int copyVariables(Component* source, Component* target)
{
	int ret = 0;
	int tempDataDid = 0; /*to track reseting did to NULL at termination*/
	const size_t maxStringSize = DYNGetMaxAuxStrLen();
	if(!dyn_allowMultipleInstances){
		if(!target->did){
			DYNGetTempDataReference(&target->did);
			tempDataDid =1;
		}
		if(!source->did){
			if(tempDataDid){
				target->did =NULL;
				return 2;
			}
			DYNGetTempDataReference(&source->did);
			tempDataDid =2;
		}
  }

  target->relativeToleranceDefined = source->relativeToleranceDefined;
  target->relativeTolerance = source->relativeTolerance;

  target->loggingOn = source->loggingOn;

  memcpy(target->states, source->states, source->nStates * sizeof(FMIReal));
  memcpy(target->derivatives, source->derivatives, source->nStates * sizeof(FMIReal));
  memcpy(target->parameters, source->parameters, source->nPar * sizeof(FMIReal));
  memcpy(target->inputs, source->inputs, source->nIn * sizeof(FMIReal));
  memcpy(target->outputs, source->outputs, source->nOut * sizeof(FMIReal));
  memcpy(target->auxiliary, source->auxiliary, source->nAux * sizeof(FMIReal));
  memcpy(target->crossingFunctions, source->crossingFunctions, source->nCross * sizeof(FMIReal));
  memcpy(target->statesNominal, source->statesNominal, source->nStates * sizeof(FMIReal));
  memcpy(target->dStates, source->dStates, source->nDStates * sizeof(FMIReal));
  memcpy(target->previousVars, source->previousVars, source->nPrevious * sizeof(FMIReal));
  if (target->DAEDerivatives && source->DAEDerivatives) memcpy(target->DAEDerivatives, source->DAEDerivatives, source->nStates * sizeof(FMIReal));
#if (FMI_VERSION >= FMI_3)
  target->nRequiredIntermediateInputs = source->nRequiredIntermediateInputs;
  target->nRequiredIntermediateOutputs = source->nRequiredIntermediateOutputs;
#endif
  {
    size_t i = 0;
    for(i = 0; i< target->nSPar; ++i) {
      memcpy(target->sParameters[i], source->sParameters[i], (maxStringSize+1) * sizeof(FMIChar));
			target->sParameters[i][maxStringSize]='\0';
    }
  }
  target->time = source->time;
  target->icall = source->icall;
  target->mStatus = source->mStatus;
  target->terminationByModel = source->terminationByModel;
  target->storeResult = source->storeResult;
  target->inlineStepCounter = source->inlineStepCounter;
  target->nextResultSampleTime = source->nextResultSampleTime;
  target->AdvancedDiscreteStates = source->AdvancedDiscreteStates;
  target->valWasSet = source->valWasSet;
  target->reinitCvode = source->reinitCvode;
  target->eventIterRequired = source->eventIterRequired;
  target->recalJacobian = source->recalJacobian;
  target->enforceRefresh = source->enforceRefresh;
  target->hycosimInputEvent = source->hycosimInputEvent;

  target->firstEventCall = source->firstEventCall;
  target->eventIterationOnGoing = source->eventIterationOnGoing;
  target->fmi2ComputeInit = source->fmi2ComputeInit;

  target->steadyStateReached = source->steadyStateReached;
  target->maxRunTimeReached = source->maxRunTimeReached;
  target->oneSimulationTotalTimerStart = source->oneSimulationTotalTimerStart;

  memcpy(target->dstruct, source->dstruct, sizeof(struct BasicDDymosimStruct));
  memcpy(target->istruct, source->istruct, sizeof(struct BasicIDymosimStruct));


  if(target->did && source->did){

	  delayStruct *srcDel = NULL, *trgDel = NULL;
	  size_t nSrcDel = 0, nTrgDel = 0, i = 0;
	  double ** trgTmpPtr = NULL;
	  double * trgTmpStatic[STATIC_TMP_ALLOC] = { 0 };
	  struct ExternalTable_ *trgTmpTable = NULL;
	  const size_t nEobj = getNbrExternalObjects();
	  const size_t sz = externalTableSize();
	  getDelayStruct(source->did, &srcDel, &nSrcDel);
	  getDelayStruct(target->did, &trgDel, &nTrgDel);
	  if (!srcDel || !trgDel || nSrcDel != nTrgDel){
		  ret = 2;
		  goto clean1;
	  }

	  if(copyDelay(trgDel, srcDel, (int)nSrcDel)){
		  ret = 1;
		  goto clean1;
	  }

	  if (nSrcDel <= STATIC_TMP_ALLOC){
		  trgTmpPtr = trgTmpStatic;
	  }
	  else{
		  trgTmpPtr = (double**) ALLOCATE_MEMORY(nSrcDel, sizeof(double*));
		  if (!trgTmpPtr){
			  ret =1;
			  goto clean1;
		}
	  }

	  for (i = 0; i < nSrcDel; ++i) {
		  trgTmpPtr[i] = trgDel[i].x;
	  }

	  /*The tabels will be copied when we copy did, we dont want that so save them before*/
	  if(nEobj){
		  struct ExternalTable_* ptr = DymosimGetExternalObject2(target->did);
		  if(!ptr){
			  ret = 2;
			  goto clean1;
		  }
		  trgTmpTable = (struct ExternalTable_*) ALLOCATE_MEMORY(nEobj,sz);
		  if(!trgTmpTable){
			ret = 1;
			goto clean1;
		  }
		  memcpy(trgTmpTable,ptr,nEobj*sz);
	  }

	  memcpy(target->did, source->did, dyn_instanceDataSize);
	  for (i = 0; i < nSrcDel; ++i) {
		  trgDel[i].x = trgTmpPtr[i];
		  trgDel[i].y = trgDel[i].x + trgDel[i].nx;
	  }
	  if(nEobj && trgTmpTable){
		  struct ExternalTable_* ptr = DymosimGetExternalObject2(target->did);
		  if(!ptr){
			  ret = 2;
			  goto clean1;
		  }
		  memcpy(ptr,trgTmpTable,nEobj*sz);
	  }
	  DYNStrClear(target->did);
clean1:
	  switch (tempDataDid)
	  {
	  case 1:
		  target->did = NULL;
		  break;
	  case 2:
		  source->did = NULL;
		  break;
	  }
	  if(nSrcDel > STATIC_TMP_ALLOC){
		  FREE_MEMORY(trgTmpPtr);
	  }
	  trgTmpPtr = NULL;
	  if(nEobj){
		FREE_MEMORY(trgTmpTable);
	  }
  }
  return ret;
}
#undef STATIC_TMP_ALLOC
#endif /* FMI_2 */

#if (FMI_VERSION >= FMI_3)
static DYM_INLINE FMIValueReference check_arrayVR(FMIValueReference vr, FMIBoolean* isArray){
	
	FMICategory cat = (FMICategory) FMI_CATEGORY(vr);
	if(fmiArray == cat){
		unsigned int index = FMI_INDEX(vr);
		if(isArray)
			*isArray = FMITrue;
		return arrayInternalValueReference[index];	
	}
	if(isArray)
		*isArray = FMIFalse;
	return vr;
}
#endif

static DYM_INLINE double* getCatPointer(Component* comp, FMICategory cat){
	switch (cat) {
		case fmiOutput:
			return comp->outputs;
		case fmiAux:
			return comp->auxiliary;
		case fmiState:
			return comp->states;
		case fmiDerivative:
			return comp->derivatives;
		case fmiParameter:
		case fmiParameter2:
			return comp->parameters;
		case fmiInput: 
		case fmiInput2:
			return comp->inputs;
		case 11: /*dState */
		case 12: /*PreviousVar*/
			return comp->dStates;
		case fmiStructuralParameter:
			return comp->dynArraySize.dim;
		case fmiResidual:
			return comp->derivatives + comp->nxOrig;
		case fmiAlgebraic:
			return comp->states + comp->nxOrig;
		default:
			return NULL;
	}
}

static DYM_INLINE FMIBoolean setIsAllowed(Component* comp, FMICategory cat, int index){
	
#if (FMI_VERSION >= FMI_3)
	if (comp->mStatus == modelConfigurationMode)
		return cat == fmiStructuralParameter ? FMITrue : FMIFalse;
#endif
	switch (cat) {
		case fmiInput:
			return FMITrue;
		case fmiState:
			return FMITrue; /*Standard doesn't allow but we do*/
		case fmiAlgebraic:
			return FMITrue;
		case fmiAux:
			/*Allow if inline since states have become aux at that time*/
			if ( (comp->mStatus == modelInstantiated || comp->mStatus == modelInitializationMode || comp->AdvancedDiscreteStates) && (size_t)index >= comp->nConstAux)
				return FMITrue;
			return FMIFalse;
		case fmiParameter:
			return FMITrue; /*Fixed parameters should not be allowed but we dont have a good way to distinguish fixed and tunable*/
		case fmiDerivative:
			if (comp->mStatus == modelInstantiated || comp->mStatus == modelInitializationMode)
				return FMITrue;
			return FMIFalse;
		case fmiOutput:
			if (comp->mStatus == modelInstantiated || comp->mStatus == modelInitializationMode 
				|| GetAdditionalFlags(40))
				return FMITrue;
			return FMIFalse;
		case fmiDState:
			return FMITrue;
		case fmiPrevious:
			return FMITrue;
		case fmiSystemVar2:
			if (comp->mStatus == modelInstantiated || comp->mStatus == modelInitializationMode)
				return FMITrue;
			return FMIFalse;
		case fmiParameter2:
		case fmiInput2:
		default:
			break;
	}
	return FMIFalse;
}
