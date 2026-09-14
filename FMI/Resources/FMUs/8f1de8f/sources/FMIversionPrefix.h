/*Header for applying correct version prefix to FMI types and Functions*/
#ifndef FMI_VERSION_PREFIX
#define FMI_VERSION_PREFIX
#define FMI_1 100
#define FMI_2 200
#define FMI_3 300

#if (FMI_VERSION >= FMI_3)
#include "fmi3PlatformTypes.h"
#include "fmi3FunctionTypes.h"
#define FMIprefix fmi3
#elif (FMI_VERSION >= FMI_2)
#include "fmi2TypesPlatform.h"
#include "fmi2FunctionTypes.h"
#define FMIprefix fmi2
#else
#include "fmiFunctions_.h"
#define FMIprefix fmi
#endif/*FMI_2*/

#define VersionPaste(a,b)     a ## b
#define VersionPasteB(a,b)    VersionPaste(a,b)
#define VersionFullName(name) VersionPasteB(FMIprefix, name)

#if (FMI_VERSION >= FMI_3)
#define FMIComponent			VersionFullName(Instance)           
#define FMICompoenetEnvironment VersionFullName(InstanceEnvironment) 
#define FMIReal					VersionFullName(Float64)
#define FMIInteger				VersionFullName(Int32)
#define FMIFMUstate				VersionFullName(FMUState)  
#else
/*-------------- fmi2TypesPlatform Conversion------------------*/
#define FMIComponent			VersionFullName(Component)               
#define FMICompoenetEnvironment VersionFullName(ComponentEnvironment) 
#define FMIReal					VersionFullName(Real)
#define FMIInteger				VersionFullName(Integer)
#define FMIFMUstate				VersionFullName(FMUstate)  
#endif
#define FMIBoolean				VersionFullName(Boolean)
#define FMIChar					VersionFullName(Char)
#define FMIString				VersionFullName(String)
#define FMIByte					VersionFullName(Byte)
#define FMITrue					VersionFullName(True)	
#define FMIFalse				VersionFullName(False)
    
#define FMIValueReference		VersionFullName(ValueReference)
/*fmiFunctions/fmi2FunctionTypes*/
/*--------------------Enumeraiton Status----------------------*/
#if (FMI_VERSION >= FMI_3)
#define FMIOK					fmi3OK
#elif (FMI_VERSION >= FMI_2)
#define FMIOK					fmi2OK
#else
#define FMIOK					fmiOK
#endif
#define FMIWarning				VersionFullName(Warning)
#define FMIDiscard				VersionFullName(Discard)
#define FMIError				VersionFullName(Error)
#define FMIFatal				VersionFullName(Fatal)
#define FMIPending				VersionFullName(Pending)
#define FMIStatus				VersionFullName(Status)

/*-------------------Enumeration Type--------------------------*/
#define FMIModelExchange        VersionFullName(ModelExchange)
#define FMICoSimulation			VersionFullName(CoSimulation)
#define FMIType					VersionFullName(Type)

#if (FMI_VERSION >= FMI_3)
typedef struct {
	FMIBoolean newDiscreteStatesNeeded;
	FMIBoolean terminateSimulation;
	FMIBoolean nominalsOfContinuousStatesChanged;
	FMIBoolean valuesOfContinuousStatesChanged;
	FMIBoolean nextEventTimeDefined;
	FMIReal    nextEventTime;
} FMIEventInfo;
#endif

#if (FMI_VERSION <= FMI_2)
/*-------------------Enumeraion StausKind----------------------*/
#define FMIDoStepStatus			VersionFullName(DoStpeStatus)
#define FMIPendingStatus		VersionFullName(PendingStatus)
#define FMILastSuccessfulTime   VersionFullName(LastSuccessfulTime)
#define FMITerminated			VersionFullName(Terminated)
#define FMIStatusKind			VersionFullName(StatusKind)
/*-------------------Struct CallBackFunctions-------------------*/
#define FMICallbackLogger			VersionFullName(CallBackLogger)
#define FMICallbackAllocateMemory	VersionFullName(CallbackAllocateMemory)
#define FMICallbackFreeMemory		VersionFullName(CallbackFreeMemory)
#define FMIStepFinished				VersionFullName(StepFinished)
#define FMICallbackFunctions		VersionFullName(CallbackFunctions)
/*-------------------Struct EventInfo---------------------------*/
#define FMIEventInfo			VersionFullName(EventInfo)
#endif
#endif /*FMI_VERSION_PREFIX*/
