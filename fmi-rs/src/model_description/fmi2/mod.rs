#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]
pub mod file;
pub mod validation;

use std::{ops::Range, str::FromStr};

use crate::{
    fmi2::types::fmi2ValueReference,
    model_description::{Category, DefaultExperiment, ModelDescriptionError, Unit},
};

pub type VariableIndex = u32;

#[derive(Debug, PartialEq)]
pub enum VariableType {
    Real {
        declaredType: Option<String>,
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        min: Option<String>,
        max: Option<String>,
        nominal: Option<String>,
        unbounded: bool,
        start: Option<String>,
        derivative: Option<VariableIndex>,
        reinit: bool,
    },
    Integer {
        declaredType: Option<String>,
        quantity: Option<String>,
        min: Option<String>,
        max: Option<String>,
        start: Option<String>,
    },
    Boolean {
        declaredType: Option<String>,
        start: Option<String>,
    },
    String {
        declaredType: Option<String>,
        start: Option<String>,
    },
    Enumeration {
        declaredType: String,
        quantity: Option<String>,
        min: Option<String>,
        max: Option<String>,
        start: Option<String>,
    },
}

impl VariableType {
    /// Returns the name of the variable type regardless of the variant.
    pub fn name(&self) -> &'static str {
        match self {
            VariableType::Real { .. } => "Real",
            VariableType::Integer { .. } => "Integer",
            VariableType::Boolean { .. } => "Boolean",
            VariableType::String { .. } => "String",
            VariableType::Enumeration { .. } => "Enumeration",
        }
    }

    /// Returns true if the start attribute is set.
    pub fn has_start(&self) -> bool {
        match self {
            VariableType::Real { start, .. } => start.is_some(),
            VariableType::Integer { start, .. } => start.is_some(),
            VariableType::Boolean { start, .. } => start.is_some(),
            VariableType::String { start, .. } => start.is_some(),
            VariableType::Enumeration { start, .. } => start.is_some(),
        }
    }

    /// Returns true if the start attribute is set.
    pub fn start(&self) -> &Option<String> {
        match self {
            VariableType::Real { start, .. } => start,
            VariableType::Integer { start, .. } => start,
            VariableType::Boolean { start, .. } => start,
            VariableType::String { start, .. } => start,
            VariableType::Enumeration { start, .. } => start,
        }
    }

    /// Returns the index of the variable this variable is the derivative of.
    pub fn derivative(&self) -> Result<u32, ModelDescriptionError> {
        if let VariableType::Real {
            derivative: Some(index),
            ..
        } = self
        {
            Ok(*index)
        } else {
            Err(ModelDescriptionError::MissingAttribute(
                "derivative".to_owned(),
            ))
        }
    }
}

#[derive(Debug, PartialEq, Clone, Copy, Eq, Hash)]
pub enum Causality {
    Parameter,
    CalculatedParameter,
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

#[derive(Debug, PartialEq, Clone, Copy, Eq, Hash)]
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
                "Unknown intial: {}",
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
    pub value: i32,
    pub description: Option<String>,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub enum SimpleType {
    Real {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        unit: Option<String>,
        displayUnit: Option<String>,
        relativeQuantity: bool,
        min: Option<f32>,
        max: Option<f32>,
        nominal: Option<f32>,
        unbounded: bool,
        range: Range<usize>,
    },
    Integer {
        name: String,
        description: Option<String>,
        quantity: Option<String>,
        min: Option<i32>,
        max: Option<i32>,
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
    Enumeration {
        name: String,
        description: Option<String>,
        items: Vec<Item>,
        quantity: Option<String>,
        range: Range<usize>,
    },
}

impl SimpleType {
    /// Returns the name of the type definition regardless of the variant.
    pub fn name(&self) -> &str {
        match self {
            SimpleType::Real { name, .. }
            | SimpleType::Integer { name, .. }
            | SimpleType::Boolean { name, .. }
            | SimpleType::String { name, .. }
            | SimpleType::Enumeration { name, .. } => name,
        }
    }
}

#[derive(Debug)]
pub struct CoSimulation {
    pub sourceFiles: Vec<String>,
    pub modelIdentifier: String,
    pub needsExecutionTool: bool,
    pub canHandleVariableCommunicationStepSize: bool,
    pub canInterpolateInputs: bool,
    pub maxOutputDerivativeOrder: u32,
    pub canRunAsynchronuously: bool,
    pub canBeInstantiatedOnlyOncePerProcess: bool,
    pub canNotUseMemoryManagementFunctions: bool,
    pub canGetAndSetFMUstate: bool,
    pub canSerializeFMUstate: bool,
    pub providesDirectionalDerivative: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct ModelExchange {
    pub sourceFiles: Vec<String>,
    pub modelIdentifier: String,
    pub needsExecutionTool: bool,
    pub completedIntegratorStepNotNeeded: bool,
    pub canBeInstantiatedOnlyOncePerProcess: bool,
    pub canNotUseMemoryManagementFunctions: bool,
    pub canGetAndSetFMUstate: bool,
    pub canSerializeFMUstate: bool,
    pub providesDirectionalDerivative: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct ScalarVariable {
    pub variableType: VariableType,
    pub name: String,
    pub valueReference: fmi2ValueReference,
    pub description: Option<String>,
    pub causality: Causality,
    pub variability: Variability,
    pub initial: Option<Initial>,
    pub canHandleMultipleSetPerTimeInstant: bool,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct Unknown {
    pub index: VariableIndex,
    pub dependencies: Option<Vec<VariableIndex>>,
    pub dependenciesKind: Option<Vec<DependencyKind>>,
    pub range: Range<usize>,
}

#[derive(Debug)]
pub struct ModelDescription {
    pub modelName: String,
    pub guid: String,
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
    pub unitDefintions: Vec<Unit>,
    pub typeDefinitions: Vec<SimpleType>,
    pub modelVariables: Vec<ScalarVariable>,
    pub numberOfEventIndicators: u32,
    pub outputs: Vec<Unknown>,
    pub derivatives: Vec<Unknown>,
    pub initialUnknowns: Vec<Unknown>,
}

impl ModelDescription {
    /// Returns the variable found with the given name.
    pub fn variable_by_name(&self, name: &str) -> Result<&ScalarVariable, ModelDescriptionError> {
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

    /// Returns the variable with the given index as a Result
    pub fn variable_by_index(
        &self,
        index: VariableIndex,
    ) -> Result<&ScalarVariable, ModelDescriptionError> {
        let Some(zero_based_index) = index.checked_sub(1) else {
            return Err(ModelDescriptionError::VariableIndex(index));
        };

        self.modelVariables
            .get(zero_based_index as usize)
            .ok_or(ModelDescriptionError::VariableIndex(index))
    }

    pub fn get_unit<'a>(&'a self, variable: &'a ScalarVariable) -> Option<&'a str> {
        if let VariableType::Real {
            unit, declaredType, ..
        } = &variable.variableType
        {
            if let Some(unit) = unit {
                return Some(unit);
            } else if let Some(declaredType) = declaredType {
                for simple_type in &self.typeDefinitions {
                    if let SimpleType::Real { name, unit, .. } = simple_type
                        && name == declaredType
                    {
                        return unit.as_deref();
                    }
                }
            }
        }
        None
    }

    pub fn get_type_definition(&self, name: &str) -> Option<&SimpleType> {
        self.typeDefinitions.iter().find(|t| t.name() == name)
    }
}
