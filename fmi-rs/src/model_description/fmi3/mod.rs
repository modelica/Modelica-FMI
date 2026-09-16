#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]
pub mod file;
pub mod validation;

use std::{ops::Range, str::FromStr};

use crate::{
    fmi3::types::fmi3ValueReference,
    model_description::{Category, DefaultExperiment, ModelDescriptionError, Unit},
};

#[derive(Debug, PartialEq, Eq, Hash)]
pub enum VariableNamingConvention {
    Flat,
    Structured,
}

impl FromStr for VariableNamingConvention {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "flat" => Ok(VariableNamingConvention::Flat),
            "structured" => Ok(VariableNamingConvention::Structured),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown variable naming convention: {}",
                s
            ))),
        }
    }
}

#[derive(Debug)]
pub struct Item {
    pub name: String,
    pub value: i64,
    pub description: Option<String>,
    pub range: Range<usize>,
}

#[derive(Debug, Clone, Copy)]
pub enum IntervalVariability {
    Constant,
    Fixed,
    Tunable,
    Changing,
    Countdown,
    Triggered,
}

impl FromStr for IntervalVariability {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "constant" => Ok(IntervalVariability::Constant),
            "fixed" => Ok(IntervalVariability::Fixed),
            "tunable" => Ok(IntervalVariability::Tunable),
            "changing" => Ok(IntervalVariability::Changing),
            "countdown" => Ok(IntervalVariability::Countdown),
            "triggered" => Ok(IntervalVariability::Triggered),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown interval variability: {}",
                s
            ))),
        }
    }
}

#[derive(Debug)]
pub enum TypeDefinition {
    Float32 {
        // fmi3TypeDefinitionBase
        name: String,
        description: Option<String>,
        // fmi3RealBaseAttributes
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        unbounded: bool,
        // fmi3Float32Attributes
        min: Option<f32>,
        max: Option<f32>,
        nominal: Option<f32>,
        range: Range<usize>,
    },
    Float64 {
        // fmi3TypeDefinitionBase
        name: String,
        description: Option<String>,
        // fmi3RealBaseAttributes
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        unbounded: bool,
        // fmi3Float64Attributes
        min: Option<f64>,
        max: Option<f64>,
        nominal: Option<f64>,
        range: Range<usize>,
    },
    Int8 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<i8>,
        max: Option<i8>,
        range: Range<usize>,
    },
    UInt8 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<u8>,
        max: Option<u8>,
        range: Range<usize>,
    },
    Int16 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<i16>,
        max: Option<i16>,
        range: Range<usize>,
    },
    UInt16 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<u16>,
        max: Option<u16>,
        range: Range<usize>,
    },
    Int32 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<i32>,
        max: Option<i32>,
        range: Range<usize>,
    },
    UInt32 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<u32>,
        max: Option<u32>,
        range: Range<usize>,
    },
    Int64 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<i64>,
        max: Option<i64>,
        range: Range<usize>,
    },
    UInt64 {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<u64>,
        max: Option<u64>,
        range: Range<usize>,
    },
    Boolean {
        name: String,
        description: Option<String>,
        range: Range<usize>,
    },
    String {
        name: String,
        description: Option<String>,
        range: Range<usize>,
    },
    Binary {
        name: String,
        description: Option<String>,
        mimeType: String,
        maxSize: Option<u64>,
        range: Range<usize>,
    },
    Enumeration {
        name: String,
        description: Option<String>,
        items: Vec<Item>,
        quantity: Option<String>,
        min: Option<i64>,
        max: Option<i64>,
        range: Range<usize>,
    },
    Clock {
        name: String,
        description: Option<String>,
        canBeDeactivated: bool,
        priority: Option<u32>,
        intervalVariability: IntervalVariability,
        intervalDecimal: Option<f64>,
        shiftDecimal: f64,
        supportsFraction: bool,
        resolution: Option<u64>,
        intervalCounter: Option<u64>,
        shiftCounter: u64,
        range: Range<usize>,
    },
}

impl TypeDefinition {
    pub fn name(&self) -> &str {
        match self {
            TypeDefinition::Float32 { name, .. }
            | TypeDefinition::Float64 { name, .. }
            | TypeDefinition::Int8 { name, .. }
            | TypeDefinition::UInt8 { name, .. }
            | TypeDefinition::Int16 { name, .. }
            | TypeDefinition::UInt16 { name, .. }
            | TypeDefinition::Int32 { name, .. }
            | TypeDefinition::UInt32 { name, .. }
            | TypeDefinition::Int64 { name, .. }
            | TypeDefinition::UInt64 { name, .. }
            | TypeDefinition::Boolean { name, .. }
            | TypeDefinition::String { name, .. }
            | TypeDefinition::Binary { name, .. }
            | TypeDefinition::Enumeration { name, .. }
            | TypeDefinition::Clock { name, .. } => name,
        }
    }
}

#[derive(Debug, Clone)]
pub enum VariableType {
    Float32 {
        start: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        declaredType: Option<String>,
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        unbounded: bool,
        min: Option<f32>,
        max: Option<f32>,
        nominal: Option<f32>,
        derivative: Option<fmi3ValueReference>,
        reinit: bool,
    },
    Float64 {
        start: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        declaredType: Option<String>,
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        unbounded: bool,
        min: Option<f64>,
        max: Option<f64>,
        nominal: Option<f64>,
        derivative: Option<fmi3ValueReference>,
        reinit: bool,
    },
    Int8 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<i8>,
        max: Option<i8>,
    },
    UInt8 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<u8>,
        max: Option<u8>,
    },
    Int16 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<i16>,
        max: Option<i16>,
    },
    UInt16 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<u16>,
        max: Option<u16>,
    },
    Int32 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<i32>,
        max: Option<i32>,
    },
    UInt32 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<u32>,
        max: Option<u32>,
    },
    Int64 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<i64>,
        max: Option<i64>,
    },
    UInt64 {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
        quantity: Option<String>,
        min: Option<u64>,
        max: Option<u64>,
    },
    Boolean {
        start: Option<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
    },
    String {
        start: Vec<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
    },
    Binary {
        start: Vec<String>,
        declaredType: Option<String>,
        intermediateUpdate: bool,
        previous: Option<u32>,
    },
    Clock {
        declaredType: Option<String>,
        intermediateUpdate: bool,
        canBeDeactivated: bool,
        priority: Option<u32>,
        intervalVariability: IntervalVariability,
        intervalDecimal: Option<f64>,
        shiftDecimal: f64,
        supportsFraction: bool,
        resolution: Option<u64>,
        intervalCounter: Option<u64>,
        shiftCounter: u64,
    },
    Enumeration {
        start: Option<String>,
        declaredType: String,
        intermediateUpdate: bool,
        previous: Option<u32>,
    },
}

impl VariableType {
    /// Returns the name of the variable type regardless of the variant.
    pub fn name(&self) -> &'static str {
        match self {
            VariableType::Float32 { .. } => "Float32",
            VariableType::Float64 { .. } => "Float64",
            VariableType::Int8 { .. } => "Int8",
            VariableType::UInt8 { .. } => "UInt8",
            VariableType::Int16 { .. } => "Int16",
            VariableType::UInt16 { .. } => "UInt16",
            VariableType::Int32 { .. } => "Int32",
            VariableType::UInt32 { .. } => "UInt32",
            VariableType::Int64 { .. } => "Int64",
            VariableType::UInt64 { .. } => "UInt64",
            VariableType::Boolean { .. } => "Boolean",
            VariableType::String { .. } => "String",
            VariableType::Binary { .. } => "Binary",
            VariableType::Clock { .. } => "Clock",
            VariableType::Enumeration { .. } => "Enumeration",
        }
    }

    /// Returns true if the start attribute is set.
    pub fn has_start(&self) -> bool {
        match self {
            VariableType::Float32 { start, .. } => start.is_some(),
            VariableType::Float64 { start, .. } => start.is_some(),
            VariableType::Int8 { start, .. } => start.is_some(),
            VariableType::UInt8 { start, .. } => start.is_some(),
            VariableType::Int16 { start, .. } => start.is_some(),
            VariableType::UInt16 { start, .. } => start.is_some(),
            VariableType::Int32 { start, .. } => start.is_some(),
            VariableType::UInt32 { start, .. } => start.is_some(),
            VariableType::Int64 { start, .. } => start.is_some(),
            VariableType::UInt64 { start, .. } => start.is_some(),
            VariableType::Boolean { start, .. } => start.is_some(),
            VariableType::String { start, .. } => !start.is_empty(),
            VariableType::Binary { start, .. } => !start.is_empty(),
            VariableType::Clock { .. } => false,
            VariableType::Enumeration { start, .. } => start.is_some(),
        }
    }

    pub fn start(&self) -> Option<Vec<String>> {
        match self {
            VariableType::Float32 { start, .. }
            | VariableType::Float64 { start, .. }
            | VariableType::Int8 { start, .. }
            | VariableType::UInt8 { start, .. }
            | VariableType::Int16 { start, .. }
            | VariableType::UInt16 { start, .. }
            | VariableType::Int32 { start, .. }
            | VariableType::UInt32 { start, .. }
            | VariableType::Int64 { start, .. }
            | VariableType::UInt64 { start, .. }
            | VariableType::Boolean { start, .. }
            | VariableType::Enumeration { start, .. } => start
                .clone()
                .map(|s| s.split_whitespace().map(|s| s.to_owned()).collect()),
            VariableType::String { start, .. } | VariableType::Binary { start, .. } => {
                if start.is_empty() {
                    None
                } else {
                    Some(start.clone())
                }
            }
            VariableType::Clock { .. } => None,
        }
    }

    pub fn previous(&self) -> Option<u32> {
        match self {
            VariableType::Float32 { previous, .. } => *previous,
            VariableType::Float64 { previous, .. } => *previous,
            VariableType::Int8 { previous, .. } => *previous,
            VariableType::UInt8 { previous, .. } => *previous,
            VariableType::Int16 { previous, .. } => *previous,
            VariableType::UInt16 { previous, .. } => *previous,
            VariableType::Int32 { previous, .. } => *previous,
            VariableType::UInt32 { previous, .. } => *previous,
            VariableType::Int64 { previous, .. } => *previous,
            VariableType::UInt64 { previous, .. } => *previous,
            VariableType::Boolean { previous, .. } => *previous,
            VariableType::String { previous, .. } => *previous,
            VariableType::Binary { previous, .. } => *previous,
            VariableType::Clock { .. } => None,
            VariableType::Enumeration { previous, .. } => *previous,
        }
    }

    /// Returns the nominal value as an f64 if the nominal attribute is set.
    pub fn nominal(&self) -> Option<f64> {
        match self {
            VariableType::Float32 { nominal, .. } => nominal.map(f64::from),
            VariableType::Float64 { nominal, .. } => *nominal,
            _ => None,
        }
    }

    /// Returns the value reference of the variable this variable is the derivative of.
    pub fn derivative(&self) -> Option<u32> {
        match self {
            VariableType::Float32 { derivative, .. } | VariableType::Float64 { derivative, .. } => {
                *derivative
            }
            _ => None,
        }
    }
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub enum Causality {
    Parameter,
    CalculatedParameter,
    StructuralParameter,
    Input,
    Output,
    Local,
    Independent,
}

impl FromStr for Causality {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "parameter" => Ok(Causality::Parameter),
            "calculatedParameter" => Ok(Causality::CalculatedParameter),
            "structuralParameter" => Ok(Causality::StructuralParameter),
            "input" => Ok(Causality::Input),
            "output" => Ok(Causality::Output),
            "local" => Ok(Causality::Local),
            "independent" => Ok(Causality::Independent),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown causality: {}",
                s
            ))),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub enum Variability {
    Constant,
    Fixed,
    Tunable,
    Discrete,
    Continuous,
}

impl FromStr for Variability {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "constant" => Ok(Variability::Constant),
            "fixed" => Ok(Variability::Fixed),
            "tunable" => Ok(Variability::Tunable),
            "discrete" => Ok(Variability::Discrete),
            "continuous" => Ok(Variability::Continuous),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown variability: {}",
                s
            ))),
        }
    }
}

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum Initial {
    Exact,
    Approx,
    Calculated,
}

impl FromStr for Initial {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "exact" => Ok(Initial::Exact),
            "approx" => Ok(Initial::Approx),
            "calculated" => Ok(Initial::Calculated),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown initial value: {}",
                s
            ))),
        }
    }
}

#[derive(Debug, PartialEq, Eq, Hash)]
pub enum DependencyKind {
    Dependent,
    Constant,
    Fixed,
    Tunable,
    Discrete,
}

impl FromStr for DependencyKind {
    type Err = ModelDescriptionError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "dependent" => Ok(DependencyKind::Dependent),
            "constant" => Ok(DependencyKind::Constant),
            "fixed" => Ok(DependencyKind::Fixed),
            "tunable" => Ok(DependencyKind::Tunable),
            "discrete" => Ok(DependencyKind::Discrete),
            _ => Err(ModelDescriptionError::Parse(format!(
                "Unknown dependency kind: {}",
                s
            ))),
        }
    }
}

#[derive(Debug)]
pub struct ModelExchange {
    pub modelIdentifier: String,
    pub needsExecutionTool: bool,
    pub canBeInstantiatedOnlyOncePerProcess: bool,
    pub canGetAndSetFMUState: bool,
    pub canSerializeFMUState: bool,
    pub providesDirectionalDerivatives: bool,
    pub providesAdjointDerivatives: bool,
    pub providesPerElementDependencies: bool,
    pub needsCompletedIntegratorStep: bool,
    pub providesEvaluateDiscreteStates: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct CoSimulation {
    pub modelIdentifier: String,
    pub needsExecutionTool: bool,
    pub canBeInstantiatedOnlyOncePerProcess: bool,
    pub canGetAndSetFMUState: bool,
    pub canSerializeFMUState: bool,
    pub providesDirectionalDerivatives: bool,
    pub providesAdjointDerivatives: bool,
    pub providesPerElementDependencies: bool,
    pub canHandleVariableCommunicationStepSize: bool,
    pub fixedInternalStepSize: Option<String>,
    pub maxOutputDerivativeOrder: u32,
    pub recommendedIntermediateInputSmoothness: i32,
    pub providesIntermediateUpdate: bool,
    pub mightReturnEarlyFromDoStep: bool,
    pub canReturnEarlyAfterIntermediateUpdate: bool,
    pub hasEventMode: bool,
    pub providesEvaluateDiscreteStates: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct ScheduledExecution {
    pub modelIdentifier: String,
    pub needsExecutionTool: bool,
    pub canBeInstantiatedOnlyOncePerProcess: bool,
    pub canGetAndSetFMUState: bool,
    pub canSerializeFMUState: bool,
    pub providesDirectionalDerivatives: bool,
    pub providesAdjointDerivatives: bool,
    pub providesPerElementDependencies: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub enum Dimension {
    Fixed { start: usize },
    Variable { valueReference: fmi3ValueReference },
}

#[derive(Debug)]
pub struct ModelVariable {
    pub variableType: VariableType,
    pub name: String,
    pub valueReference: fmi3ValueReference,
    pub description: Option<String>,
    pub causality: Causality,
    pub variability: Variability,
    pub canHandleMultipleSetPerTimeInstant: bool,
    pub clocks: Vec<fmi3ValueReference>,
    pub initial: Option<Initial>,
    pub dimensions: Vec<Dimension>,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct Unknown {
    pub valueReference: fmi3ValueReference,
    pub dependencies: Option<Vec<fmi3ValueReference>>,
    pub dependenciesKind: Option<Vec<DependencyKind>>,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct ModelDescription {
    pub fmiVersion: String,
    pub modelName: String,
    pub instantiationToken: String,
    pub description: Option<String>,
    pub author: Option<String>,
    pub version: Option<String>,
    pub copyright: Option<String>,
    pub license: Option<String>,
    pub generationTool: Option<String>,
    pub generationDateAndTime: Option<String>,
    pub variableNamingConvention: VariableNamingConvention,
    pub logCategories: Vec<Category>,
    pub defaultExperiment: Option<DefaultExperiment>,
    pub modelExchange: Option<ModelExchange>,
    pub coSimulation: Option<CoSimulation>,
    pub scheduledExecution: Option<ScheduledExecution>,
    pub unitDefinitions: Vec<Unit>,
    pub typeDefinitions: Vec<TypeDefinition>,
    pub modelVariables: Vec<ModelVariable>,
    pub outputs: Vec<Unknown>,
    pub derivatives: Vec<Unknown>,
    pub clockedStates: Vec<Unknown>,
    pub eventIndicators: Vec<Unknown>,
    pub initialUnknowns: Vec<Unknown>,
}

impl ModelDescription {
    /// Returns the variable with the given value reference.
    pub fn variable_by_value_reference(
        &self,
        vr: fmi3ValueReference,
    ) -> Result<&ModelVariable, ModelDescriptionError> {
        self.modelVariables
            .iter()
            .find(|v| v.valueReference == vr)
            .ok_or(ModelDescriptionError::ValueReference(vr))
    }

    /// Returns the variable with the given name.
    pub fn variable_by_name(&self, name: &str) -> Result<&ModelVariable, ModelDescriptionError> {
        self.modelVariables
            .iter()
            .find(|v| v.name == name)
            .ok_or_else(|| ModelDescriptionError::VariableName(name.to_owned()))
    }

    /// Returns the index for the given variable name
    pub fn variable_index_by_name(&self, name: &str) -> Result<usize, ModelDescriptionError> {
        self.modelVariables
            .iter()
            .position(|v| v.name == name)
            .ok_or_else(|| ModelDescriptionError::VariableName(name.to_owned()))
    }

    /// Returns the initial size of each dimension of a model variable.
    pub fn initial_size(
        &self,
        variable: &ModelVariable,
    ) -> Result<Vec<usize>, ModelDescriptionError> {
        variable
            .dimensions
            .iter()
            .map(|dimension| match dimension {
                Dimension::Fixed { start } => Ok(*start),
                Dimension::Variable { valueReference } => {
                    let dimension_variable = self.variable_by_value_reference(*valueReference)?;
                    let start = match &dimension_variable.variableType {
                        VariableType::UInt64 { start, .. } => start.as_deref(),
                        _ => {
                            return Err(ModelDescriptionError::Parse(format!(
                                "Dimension variable '{}' is not UInt64",
                                dimension_variable.name
                            )));
                        }
                    }
                    .ok_or_else(|| ModelDescriptionError::MissingAttribute("start".to_owned()))?;

                    start.parse::<usize>().map_err(|error| {
                        ModelDescriptionError::Parse(format!(
                            "Invalid initial value for dimension variable '{}': {error}",
                            dimension_variable.name
                        ))
                    })
                }
            })
            .collect()
    }

    pub fn get_unit<'a>(&'a self, variable: &'a ModelVariable) -> Option<&'a str> {
        if let VariableType::Float32 {
            unit, declaredType, ..
        }
        | VariableType::Float64 {
            unit, declaredType, ..
        } = &variable.variableType
        {
            if let Some(unit) = unit {
                return Some(unit);
            } else if let Some(declaredType) = declaredType {
                for type_definition in &self.typeDefinitions {
                    if let TypeDefinition::Float32 { name, unit, .. }
                    | TypeDefinition::Float64 { name, unit, .. } = type_definition
                        && name == declaredType
                    {
                        return unit.as_deref();
                    }
                }
            }
        }
        None
    }

    pub fn get_type_definition(&self, name: &str) -> Option<&TypeDefinition> {
        self.typeDefinitions.iter().find(|t| t.name() == name)
    }
}
