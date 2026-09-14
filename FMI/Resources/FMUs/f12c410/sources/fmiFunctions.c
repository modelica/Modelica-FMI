#ifndef BUILDFMU
/* needed for source code export */
#include "conf.h"
#endif
#include "FMIversionPrefix.h"
#include "fmiFunctions_.h"
#include "fmiFunctions_fwd.h"
/* This file contains wrappers for all FMI functions and is provided in
   the Dymola distribution instead of the actual implementation. This is
   necessary to avoid exposing the latter. */

DYMOLA_STATIC void __enableResultStoring(fmiComponent c);

/*Temp hack for directional derivatives of clocked systems*/
DYMOLA_STATIC const int  fmiClockedStates = 0;
/***************************************************
 Common Functions
****************************************************/



/* ------------------------------------------------------ */
const char* fmiGetVersion()
{
	return fmiGetVersion_();
}

/* ------------------------------------------------------ */
fmiStatus fmiSetDebugLogging(fmiComponent c, fmiBoolean loggingOn
#if (FMI_VERSION >= FMI_2)
, size_t nCategories, const fmiString categories[]	
#endif
)
{
	return fmiSetDebugLogging_(c, loggingOn
#if (FMI_VERSION >= FMI_2)
                               , nCategories, categories
#endif
);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[])
{
	return fmiGetReal_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[])
{
	return fmiGetInteger_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[])
{
	return fmiGetBoolean_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[])
{
	return fmiGetString_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[])
{
	return fmiSetReal_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[])
{
	return fmiSetInteger_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[])
{
	return fmiSetBoolean_(c, vr, nvr, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[])
{
	return fmiSetString_(c, vr, nvr, value);
}


/***************************************************
 Functions for FMI for Model Exchange
****************************************************/

#ifndef FMU_SKIP_MODEL_EXCHANGE


const char* fmiGetModelTypesPlatform()
{
	return fmiGetModelTypesPlatform_();
}



fmiComponent fmiInstantiateModel(fmiString instanceName,
								 fmiString GUID,
								 fmiMECallbackFunctions functions,
								 fmiBoolean loggingOn)
{
	fmiComponent comp = fmiInstantiateModel_(instanceName, GUID, functions, loggingOn);
#ifndef FMU_SOURCE_CODE_EXPORT
	if (comp != NULL) {
		/* determine whether to store results or not */
#include "dsmodel_fmuconf.h"	
	}
#endif /* FMU_SOURCE_CODE_EXPORT */
	return comp;
}


/* ------------------------------------------------------ */
void fmiFreeModelInstance(fmiComponent c)
{
	fmiFreeModelInstance_(c);
}
/* ------------------------------------------------------ */
fmiStatus fmiSetTime(fmiComponent c, fmiReal time)
{
	return fmiSetTime_(c, time);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetContinuousStates(fmiComponent c, const fmiReal x[], size_t nx)
{
	return fmiSetContinuousStates_(c, x, nx);
}

/* ------------------------------------------------------ */


/* ------------------------------------------------------ */
fmiStatus fmiInitialize(fmiComponent c,
                        fmiBoolean toleranceControlled,
                        fmiReal relativeTolerance,
						fmiEventInfo* eventInfo)
{
	return fmiInitialize_(c, toleranceControlled, relativeTolerance, eventInfo);
}

/* ------------------------------------------------------ */
fmiStatus fmiEventUpdate(fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo* eventInfo)
{
  return fmiEventUpdate_(c, intermediateResults, eventInfo);
}

/* ------------------------------------------------------ */
fmiStatus fmiTerminate(fmiComponent c)
{
	return fmiTerminate_(c);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetStateValueReferences(fmiComponent c, fmiValueReference vrx[], size_t nx)
{
	return fmiGetStateValueReferences_(c, vrx, nx);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetNominalContinuousStates(fmiComponent c, fmiReal x_nominal[], size_t nx)
{
	return fmiGetNominalContinuousStates_(c, x_nominal, nx);
}

/* ------------------------------------------------------ */


fmiStatus fmiCompletedIntegratorStep(fmiComponent c, fmiBoolean* callEventUpdate)
{
  return fmiCompletedIntegratorStep_(c, callEventUpdate);
}


/* ------------------------------------------------------ */
fmiStatus fmiGetDerivatives(fmiComponent c, fmiReal derivatives[], size_t nx)
{
	return fmiGetDerivatives_(c, derivatives, nx);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetEventIndicators(fmiComponent c, fmiReal eventIndicators[], size_t ni)
{
	return fmiGetEventIndicators_(c, eventIndicators, ni);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetContinuousStates(fmiComponent c, fmiReal states[], size_t nx)
{
	return fmiGetContinuousStates_(c, states, nx);
}

#endif /* FMU_SKIP_MODEL_EXCHANGE */

/***************************************************
 Functions for FMI for Co-Simulation
****************************************************/

#ifndef FMU_SKIP_CO_SIM


const char* fmiGetTypesPlatform()
{
	return fmiGetTypesPlatform_();
}



fmiComponent fmiInstantiateSlave(fmiString instanceName,
                                 fmiString fmuGUID,
                                 fmiString fmuLocation,
                                 fmiString mimeType,
                                 fmiReal timeout,
                                 fmiBoolean visible,
                                 fmiBoolean interactive,
                                 fmiCallbackFunctions functions,
                                 fmiBoolean loggingOn)
{
  fmiComponent c = fmiInstantiateSlave_(instanceName,
                                        fmuGUID,
                                        fmuLocation,
                                        mimeType,
                                        timeout,
                                        visible,
                                        interactive,
                                        functions,
                                        loggingOn);

#ifdef GenerateResultInNonDymosim
	__enableResultStoring(c);
#endif
	return c;
}

/* ------------------------------------------------------ */
void fmiFreeSlaveInstance(fmiComponent c)
{
    fmiFreeSlaveInstance_(c);
}

/* ------------------------------------------------------ */

fmiStatus fmiInitializeSlave(fmiComponent c,
                             fmiReal      tStart,
                             fmiBoolean   StopTimeDefined,
                             fmiReal      tStop)
{
    return fmiInitializeSlave_(c, tStart, StopTimeDefined, tStop);
}

/* ------------------------------------------------------ */
fmiStatus fmiTerminateSlave(fmiComponent c)
{
    return fmiTerminateSlave_(c);
}

/* ------------------------------------------------------ */
fmiStatus fmiResetSlave(fmiComponent c)
{
    return fmiResetSlave_(c);
}

/* ------------------------------------------------------ */
fmiStatus fmiSetRealInputDerivatives(fmiComponent c,
                                     const  fmiValueReference vr[],
                                     size_t nvr,
                                     const  fmiInteger order[],
                                     const  fmiReal value[])
{
    return fmiSetRealInputDerivatives_(c, vr, nvr, order, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetRealOutputDerivatives(fmiComponent c,
                                      const   fmiValueReference vr[],
                                      size_t  nvr,
                                      const   fmiInteger order[],
                                      fmiReal value[])
{
    return fmiGetRealOutputDerivatives_(c, vr, nvr, order, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiDoStep(fmiComponent c,
                    fmiReal      currentCommunicationPoint,
                    fmiReal      communicationStepSize,
                    /* For 1.0: newStep,
                       For 2.0: noSetFMUStatePriorToCurrentPoint */                                fmiBoolean   genericBool)
{
    return fmiDoStep_(c, currentCommunicationPoint, communicationStepSize, genericBool);
}

/* ------------------------------------------------------ */
fmiStatus fmiCancelStep(fmiComponent c)
{
    return fmiCancelStep_(c);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetStatus(fmiComponent c, const fmiStatusKind s, fmiStatus*  value)
{
    return fmiGetStatus_(c, s, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetRealStatus(fmiComponent c, const fmiStatusKind s, fmiReal*    value)
{
    return fmiGetRealStatus_(c, s, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetIntegerStatus(fmiComponent c, const fmiStatusKind s, fmiInteger* value)
{
    return fmiGetIntegerStatus_(c, s, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s, fmiBoolean* value)
{
    return fmiGetBooleanStatus_(c, s, value);
}

/* ------------------------------------------------------ */
fmiStatus fmiGetStringStatus(fmiComponent c, const fmiStatusKind s, fmiString*  value)
{
    return fmiGetStringStatus_(c, s, value);
}

#endif /* FMU_SKIP_CO_SIM */

