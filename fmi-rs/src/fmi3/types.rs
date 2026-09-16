#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]

use std::{
    fmt,
    os::raw::{c_char, c_uint, c_void},
};

pub type fmi3Float32 = f32;
pub type fmi3Float64 = f64;
pub type fmi3Int8 = i8;
pub type fmi3UInt8 = u8;
pub type fmi3Int16 = i16;
pub type fmi3UInt16 = u16;
pub type fmi3Int32 = i32;
pub type fmi3UInt32 = u32;
pub type fmi3Int64 = i64;
pub type fmi3UInt64 = u64;
pub type fmi3Boolean = bool;
pub type fmi3Char = c_char;
pub type fmi3String = *const c_char;
pub type fmi3Byte = u8;
pub type fmi3Binary = *const fmi3Byte;
pub type fmi3Clock = bool;
pub type fmi3ValueReference = u32;
pub type fmi3FMUState = *mut c_void;
pub type fmi3Instance = *mut c_void;
pub type fmi3InstanceEnvironment = *mut c_void;

pub const fmi3True: fmi3Boolean = true;
pub const fmi3False: fmi3Boolean = false;

#[repr(i32)]
#[derive(PartialEq, PartialOrd, Clone, Eq, Ord, Copy)]
pub enum fmi3Status {
    Ok = 0,
    Warning = 1,
    Discard = 2,
    Error = 3,
    Fatal = 4,
}

impl fmt::Debug for fmi3Status {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(match self {
            Self::Ok => "fmi3OK",
            Self::Warning => "fmi3Warning",
            Self::Discard => "fmi3Discard",
            Self::Error => "fmi3Error",
            Self::Fatal => "fmi3Fatal",
        })
    }
}

impl TryFrom<i32> for fmi3Status {
    type Error = ();

    fn try_from(value: i32) -> Result<Self, ()> {
        match value {
            0 => Ok(fmi3Status::Ok),
            1 => Ok(fmi3Status::Warning),
            2 => Ok(fmi3Status::Discard),
            3 => Ok(fmi3Status::Error),
            4 => Ok(fmi3Status::Fatal),
            _ => Err(()),
        }
    }
}

#[repr(i32)]
#[derive(PartialEq, Clone, Copy)]
pub enum fmi3DependencyKind {
    Independent = 0,
    Constant = 1,
    Fixed = 2,
    Tunable = 3,
    Discrete = 4,
    Dependent = 5,
}

impl fmt::Debug for fmi3DependencyKind {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(match self {
            Self::Independent => "fmi3Independent",
            Self::Constant => "fmi3Constant",
            Self::Fixed => "fmi3Fixed",
            Self::Tunable => "fmi3Tunable",
            Self::Discrete => "fmi3Discrete",
            Self::Dependent => "fmi3Dependent",
        })
    }
}

#[repr(i32)]
#[derive(PartialEq, Clone, Copy)]
pub enum fmi3IntervalQualifier {
    IntervalNotYetKnown = 0,
    IntervalUnchanged = 1,
    IntervalChanged = 2,
}

impl fmt::Debug for fmi3IntervalQualifier {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(match self {
            Self::IntervalNotYetKnown => "fmi3IntervalNotYetKnown",
            Self::IntervalUnchanged => "fmi3IntervalUnchanged",
            Self::IntervalChanged => "fmi3IntervalChanged",
        })
    }
}

pub type fmi3LogMessageCallback = unsafe extern "C" fn(
    instanceEnvironment: fmi3InstanceEnvironment,
    status: fmi3Status,
    category: fmi3String,
    message: fmi3String,
);

pub type fmi3ClockUpdateCallback =
    unsafe extern "C" fn(instanceEnvironment: fmi3InstanceEnvironment);

pub type fmi3IntermediateUpdateCallback = unsafe extern "C" fn(
    instanceEnvironment: fmi3InstanceEnvironment,
    intermediateUpdateTime: fmi3Float64,
    intermediateVariableSetRequested: fmi3Boolean,
    intermediateVariableGetAllowed: fmi3Boolean,
    intermediateStepFinished: fmi3Boolean,
    canReturnEarly: fmi3Boolean,
    earlyReturnRequested: *mut fmi3Boolean,
    earlyReturnTime: *mut fmi3Float64,
);

pub type fmi3LockPreemptionCallback = unsafe extern "C" fn();

pub type fmi3UnlockPreemptionCallback = unsafe extern "C" fn();

// FMI 3.0 Common API function types
pub type fmi3GetVersionTYPE = unsafe extern "C" fn() -> fmi3String;

pub type fmi3SetDebugLoggingTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    loggingOn: fmi3Boolean,
    nCategories: usize,
    categories: *const fmi3String,
) -> fmi3Status;

// FMI 3.0 Creation and Destruction
pub type fmi3InstantiateModelExchangeTYPE = unsafe extern "C" fn(
    instanceName: fmi3String,
    instantiationToken: fmi3String,
    resourcePath: fmi3String,
    visible: fmi3Boolean,
    loggingOn: fmi3Boolean,
    instanceEnvironment: fmi3InstanceEnvironment,
    logMessage: *const fmi3LogMessageCallback,
) -> fmi3Instance;

pub type fmi3InstantiateCoSimulationTYPE = unsafe extern "C" fn(
    instanceName: *const c_char,
    instantiationToken: *const c_char,
    resourcePath: *const c_char,
    visible: bool,
    loggingOn: bool,
    eventModeUsed: bool,
    earlyReturnAllowed: bool,
    requiredIntermediateVariables: *const c_uint,
    nRequiredIntermediateVariables: usize,
    instanceEnvironment: fmi3InstanceEnvironment,
    logMessage: *const fmi3LogMessageCallback,
    intermediateUpdate: *const fmi3IntermediateUpdateCallback,
) -> fmi3Instance;

pub type fmi3InstantiateScheduledExecutionTYPE = unsafe extern "C" fn(
    instanceName: fmi3String,
    instantiationToken: fmi3String,
    resourcePath: fmi3String,
    visible: fmi3Boolean,
    loggingOn: fmi3Boolean,
    instanceEnvironment: fmi3InstanceEnvironment,
    logMessage: *const fmi3LogMessageCallback,
    clockUpdate: *const fmi3ClockUpdateCallback,
    lockPreemption: *const fmi3LockPreemptionCallback,
    unlockPreemption: *const fmi3UnlockPreemptionCallback,
) -> fmi3Instance;

pub type fmi3FreeInstanceTYPE = unsafe extern "C" fn(instance: fmi3Instance);

// FMI 3.0 Initialization, Termination, and Reset
pub type fmi3EnterInitializationModeTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    toleranceDefined: fmi3Boolean,
    tolerance: fmi3Float64,
    startTime: fmi3Float64,
    stopTimeDefined: fmi3Boolean,
    stopTime: fmi3Float64,
) -> fmi3Status;

pub type fmi3ExitInitializationModeTYPE =
    unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3EnterEventModeTYPE = unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3EnterStepModeTYPE = unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3TerminateTYPE = unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3ResetTYPE = unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

// FMI 3.0 Getting and Setting Variable Values
pub type fmi3GetFloat32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Float32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetFloat64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const c_uint,
    nValueReferences: usize,
    values: *mut fmi3Float64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetInt8TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Int8,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetUInt8TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3UInt8,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetInt16TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Int16,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetUInt16TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3UInt16,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetInt32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Int32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetUInt32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3UInt32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetInt64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Int64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetUInt64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3UInt64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetBooleanTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Boolean,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetStringTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3String,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetBinaryTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    sizes: *mut usize,
    values: *mut fmi3Binary,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetClockTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *mut fmi3Clock,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetFloat32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Float32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetFloat64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const c_uint,
    nValueReferences: usize,
    values: *const fmi3Float64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetInt8TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Int8,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetUInt8TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3UInt8,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetInt16TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Int16,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetUInt16TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3UInt16,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetInt32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Int32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetUInt32TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3UInt32,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetInt64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Int64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetUInt64TYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3UInt64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetBooleanTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Boolean,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetStringTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3String,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetBinaryTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    sizes: *const usize,
    values: *const fmi3Binary,
    nValues: usize,
) -> fmi3Status;

pub type fmi3SetClockTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    values: *const fmi3Clock,
    nValues: usize,
) -> fmi3Status;

// FMI 3.0 Getting Variable Dependency Information
pub type fmi3GetNumberOfVariableDependenciesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReference: fmi3ValueReference,
    nDependencies: *mut usize,
) -> fmi3Status;

pub type fmi3GetVariableDependenciesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReference: fmi3ValueReference,
    elementIndicesOfDependent: *mut usize,
    independentVariables: *mut fmi3ValueReference,
    elementIndicesOfIndependents: *mut usize,
    dependencyKinds: *mut fmi3DependencyKind,
) -> fmi3Status;

// FMI 3.0 Getting and Setting the Complete FMU State
pub type fmi3GetFMUStateTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, FMUState: *mut fmi3FMUState) -> fmi3Status;

pub type fmi3SetFMUStateTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, FMUState: fmi3FMUState) -> fmi3Status;

pub type fmi3FreeFMUStateTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, FMUState: *mut fmi3FMUState) -> fmi3Status;

pub type fmi3SerializedFMUStateSizeTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    FMUState: fmi3FMUState,
    size: *mut usize,
) -> fmi3Status;

pub type fmi3SerializeFMUStateTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    FMUState: fmi3FMUState,
    serializedState: *mut fmi3Byte,
    size: usize,
) -> fmi3Status;

pub type fmi3DeserializeFMUStateTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    serializedState: *const fmi3Byte,
    size: usize,
    FMUState: *mut fmi3FMUState,
) -> fmi3Status;

// FMI 3.0 Getting Partial Derivatives
pub type fmi3GetDirectionalDerivativeTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    unknowns: *const fmi3ValueReference,
    nUnknowns: usize,
    knowns: *const fmi3ValueReference,
    nKnowns: usize,
    seed: *const fmi3Float64,
    nSeed: usize,
    sensitivity: *mut fmi3Float64,
    nSensitivity: usize,
) -> fmi3Status;

pub type fmi3GetAdjointDerivativeTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    unknowns: *const fmi3ValueReference,
    nUnknowns: usize,
    knowns: *const fmi3ValueReference,
    nKnowns: usize,
    seed: *const fmi3Float64,
    nSeed: usize,
    sensitivity: *mut fmi3Float64,
    nSensitivity: usize,
) -> fmi3Status;

pub type fmi3EnterConfigurationModeTYPE =
    unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3ExitConfigurationModeTYPE = unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

// FMI 3.0 Co-Simulation specific functions
pub type fmi3DoStepTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    currentCommunicationPoint: fmi3Float64,
    communicationStepSize: fmi3Float64,
    noSetFMUStatePriorToCurrentPoint: fmi3Boolean,
    eventHandlingNeeded: *mut fmi3Boolean,
    terminateSimulation: *mut fmi3Boolean,
    earlyReturn: *mut fmi3Boolean,
    lastSuccessfulTime: *mut fmi3Float64,
) -> fmi3Status;

pub type fmi3ActivateModelPartitionTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    clockReference: fmi3ValueReference,
    activationTime: fmi3Float64,
    priority: fmi3Float64,
) -> fmi3Status;

// FMI 3.0 Model Exchange specific functions
pub type fmi3EnterContinuousTimeModeTYPE =
    unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;

pub type fmi3CompletedIntegratorStepTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    noSetFMUStatePriorToCurrentPoint: fmi3Boolean,
    enterEventMode: *mut fmi3Boolean,
    terminateSimulation: *mut fmi3Boolean,
) -> fmi3Status;

pub type fmi3SetTimeTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, time: fmi3Float64) -> fmi3Status;

pub type fmi3SetContinuousStatesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    continuousStates: *const fmi3Float64,
    nContinuousStates: usize,
) -> fmi3Status;

pub type fmi3GetContinuousStatesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    continuousStates: *mut fmi3Float64,
    nContinuousStates: usize,
) -> fmi3Status;

pub type fmi3GetContinuousStateDerivativesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    derivatives: *mut fmi3Float64,
    nDerivatives: usize,
) -> fmi3Status;

pub type fmi3GetEventIndicatorsTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    eventIndicators: *mut fmi3Float64,
    nEventIndicators: usize,
) -> fmi3Status;

pub type fmi3GetContinuousStatesChangedTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, statesChanged: *mut fmi3Boolean) -> fmi3Status;

pub type fmi3GetNominalsOfContinuousStatesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    nominals: *mut fmi3Float64,
    nNominals: usize,
) -> fmi3Status;

pub type fmi3GetNumberOfEventIndicatorsTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, nEventIndicators: *mut usize) -> fmi3Status;

pub type fmi3GetNumberOfContinuousStatesTYPE =
    unsafe extern "C" fn(instance: fmi3Instance, nContinuousStates: *mut usize) -> fmi3Status;

// FMI 3.0 Scheduled Execution specific functions
pub type fmi3CallbackIntermediateUpdateTYPE = unsafe extern "C" fn(
    instanceEnvironment: fmi3InstanceEnvironment,
    intermediateUpdateTime: fmi3Float64,
    eventOccurred: fmi3Boolean,
    clocksTicked: fmi3Boolean,
    intermediateVariableSetRequested: fmi3Boolean,
    intermediateVariableGetAllowed: fmi3Boolean,
    intermediateStepFinished: fmi3Boolean,
    canReturnEarly: fmi3Boolean,
    earlyReturnRequested: *mut fmi3Boolean,
    earlyReturnTime: *mut fmi3Float64,
) -> fmi3Status;

pub type fmi3UpdateDiscreteStatesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    discreteStatesNeedUpdate: *mut fmi3Boolean,
    terminateSimulation: *mut fmi3Boolean,
    nominalsOfContinuousStatesChanged: *mut fmi3Boolean,
    valuesOfContinuousStatesChanged: *mut fmi3Boolean,
    nextEventTimeDefined: *mut fmi3Boolean,
    nextEventTime: *mut fmi3Float64,
) -> fmi3Status;

// Additional FMI 3.0 functions that might be missing
pub type fmi3GetOutputDerivativesTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    orders: *const fmi3Int32,
    values: *mut fmi3Float64,
    nValues: usize,
) -> fmi3Status;

pub type fmi3GetIntervalDecimalTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    intervals: *mut fmi3Float64,
    qualifiers: *mut fmi3IntervalQualifier,
) -> fmi3Status;

pub type fmi3GetIntervalFractionTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    counters: *mut fmi3UInt64,
    resolutions: *mut fmi3UInt64,
    qualifiers: *mut fmi3IntervalQualifier,
) -> fmi3Status;

pub type fmi3GetShiftDecimalTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    shifts: *mut fmi3Float64,
) -> fmi3Status;

pub type fmi3GetShiftFractionTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    counters: *mut fmi3UInt64,
    resolutions: *mut fmi3UInt64,
) -> fmi3Status;

pub type fmi3SetIntervalDecimalTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    intervals: *const fmi3Float64,
) -> fmi3Status;

pub type fmi3SetIntervalFractionTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    counters: *const fmi3UInt64,
    resolutions: *const fmi3UInt64,
) -> fmi3Status;

pub type fmi3SetShiftDecimalTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    shifts: *const fmi3Float64,
) -> fmi3Status;

pub type fmi3SetShiftFractionTYPE = unsafe extern "C" fn(
    instance: fmi3Instance,
    valueReferences: *const fmi3ValueReference,
    nValueReferences: usize,
    counters: *const fmi3UInt64,
    resolutions: *const fmi3UInt64,
) -> fmi3Status;

pub type fmi3EvaluateDiscreteStatesTYPE =
    unsafe extern "C" fn(instance: fmi3Instance) -> fmi3Status;
