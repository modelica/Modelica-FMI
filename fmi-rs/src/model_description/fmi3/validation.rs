use std::collections::{HashMap, HashSet};

use crate::{
    fmi3::types::fmi3ValueReference,
    model_description::{
        ValidationError,
        fmi3::{
            Causality, Initial, ModelDescription, ModelVariable, Unknown, Variability,
            VariableNamingConvention, VariableType,
        },
        validation::validate_structured_variable_name,
    },
};

impl ModelDescription {
    pub fn validate(&self) -> Vec<ValidationError> {
        let mut problems = vec![];

        let mut independent_variable: Option<&ModelVariable> = None;
        let mut value_references: HashMap<u32, &ModelVariable> = HashMap::new();
        let mut variable_names: HashMap<&String, &ModelVariable> = HashMap::new();

        if self.modelExchange.is_none()
            && self.coSimulation.is_none()
            && self.scheduledExecution.is_none()
        {
            problems.push(ValidationError {
                range: vec![],
                message: "No interface type defined.".to_string(),
            });
        }

        // validate model variables
        for variable in &self.modelVariables {
            // validate variable name
            if variable.name.is_empty() {
                problems.push(ValidationError {
                    range: vec![variable.range.clone()],
                    message: "Variable name cannot be empty.".to_string(),
                });
            } else if self.variableNamingConvention == VariableNamingConvention::Structured
                && let Err(message) = validate_structured_variable_name(&variable.name)
            {
                problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: format!("Variable name '{}' does not conform to the structured naming convention: {}", variable.name, message),
                    });
            }

            // check for duplicate value references
            if let Some(duplicate) = value_references.get(&variable.valueReference) {
                problems.push(ValidationError {
                    range: vec![duplicate.range.clone(), variable.range.clone()],
                    message: format!("Duplicate value reference: {}", variable.valueReference),
                });
            } else {
                value_references.insert(variable.valueReference, variable);
            }

            // check for duplicate variable names
            if let Some(duplicate) = variable_names.get(&variable.name) {
                problems.push(ValidationError {
                    range: vec![duplicate.range.clone(), variable.range.clone()],
                    message: format!("Duplicate variable name: '{}'", variable.name),
                });
            } else {
                variable_names.insert(&variable.name, variable);
            }

            // assert no start for variability "calculated"
            if let Some(Initial::Calculated) = variable.initial
                && variable.variableType.has_start()
            {
                problems.push(ValidationError {
                    range: vec![variable.range.clone()],
                    message: format!(
                        "Variable '{}' is calculated but provides a start value.",
                        variable.name
                    ),
                });
            }

            // assert required start values
            if !matches!(variable.variableType, VariableType::Clock { .. }) {
                let is_exact_or_approx = matches!(
                    variable.initial,
                    Some(Initial::Exact) | Some(Initial::Approx)
                );

                if (is_exact_or_approx || variable.causality == Causality::Input)
                    && !variable.variableType.has_start()
                {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: format!("Variable '{}' has no start value.", variable.name),
                    });
                }
            }

            // check independent variable
            if variable.causality == Causality::Independent {
                if let Some(independent_variable) = independent_variable {
                    problems.push(ValidationError {
                        range: vec![independent_variable.range.clone(), variable.range.clone()],
                        message: "There can only be one independent variable.".to_string(),
                    });
                } else {
                    independent_variable = Some(variable);
                }

                if !matches!(
                    variable.variableType,
                    VariableType::Float32 { .. } | VariableType::Float64 { .. }
                ) {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: "The independent variable must be a Float32 or Float64 variable."
                            .to_string(),
                    });
                }

                if variable.variability != Variability::Continuous {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: "The independent variable must be continuous.".to_string(),
                    });
                }

                if variable.variableType.has_start() {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: "The independent variable must not have a start value."
                            .to_string(),
                    });
                }
            }

            // check combination of causality and variability, and initial
            match (
                &variable.causality,
                &variable.variability,
                &variable.initial,
            ) {
                (Causality::StructuralParameter, Variability::Fixed, Some(Initial::Exact)) => {}
                (Causality::StructuralParameter, Variability::Tunable, Some(Initial::Exact)) => {}
                (Causality::Parameter, Variability::Fixed, Some(Initial::Exact)) => {}
                (Causality::Parameter, Variability::Tunable, Some(Initial::Exact)) => {}
                (Causality::CalculatedParameter, Variability::Fixed, Some(Initial::Calculated)) => {
                }
                (Causality::CalculatedParameter, Variability::Fixed, Some(Initial::Approx)) => {}
                (
                    Causality::CalculatedParameter,
                    Variability::Tunable,
                    Some(Initial::Calculated),
                ) => {}
                (Causality::CalculatedParameter, Variability::Tunable, Some(Initial::Approx)) => {}
                (Causality::Input, Variability::Discrete, Some(Initial::Exact)) => {}
                (Causality::Input, Variability::Continuous, Some(Initial::Exact)) => {}
                (Causality::Output, Variability::Constant, Some(Initial::Exact)) => {}
                (Causality::Output, Variability::Discrete, Some(Initial::Calculated)) => {}
                (Causality::Output, Variability::Discrete, Some(Initial::Exact)) => {}
                (Causality::Output, Variability::Discrete, Some(Initial::Approx)) => {}
                (Causality::Output, Variability::Continuous, Some(Initial::Calculated)) => {}
                (Causality::Output, Variability::Continuous, Some(Initial::Exact)) => {}
                (Causality::Output, Variability::Continuous, Some(Initial::Approx)) => {}
                (Causality::Local, Variability::Constant, Some(Initial::Exact)) => {}
                (Causality::Local, Variability::Fixed, Some(Initial::Calculated)) => {}
                (Causality::Local, Variability::Fixed, Some(Initial::Approx)) => {}
                (Causality::Local, Variability::Tunable, Some(Initial::Calculated)) => {}
                (Causality::Local, Variability::Tunable, Some(Initial::Approx)) => {}
                (Causality::Local, Variability::Discrete, Some(Initial::Calculated)) => {}
                (Causality::Local, Variability::Discrete, Some(Initial::Exact)) => {}
                (Causality::Local, Variability::Discrete, Some(Initial::Approx)) => {}
                (Causality::Local, Variability::Continuous, Some(Initial::Calculated)) => {}
                (Causality::Local, Variability::Continuous, Some(Initial::Exact)) => {}
                (Causality::Local, Variability::Continuous, Some(Initial::Approx)) => {}
                (Causality::Independent, Variability::Continuous, None) => {}
                _ => {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: format!(
                            "Illegal combination of causality '{:?}', variability '{:?}', and initial '{:?}' for variable '{}'.",
                            variable.causality, variable.variability, variable.initial, variable.name
                        ),
                    });
                }
            }
        }

        // validate outputs
        for unknown in &self.outputs {
            problems.extend(self.validate_unknown(unknown));
        }

        let expected_output_vrs = self
            .modelVariables
            .iter()
            .filter(|v| v.causality == Causality::Output)
            .map(|v| v.valueReference)
            .collect::<HashSet<u32>>();

        let actual_output_vrs = self
            .outputs
            .iter()
            .map(|u| u.valueReference)
            .collect::<HashSet<u32>>();

        if expected_output_vrs != actual_output_vrs {
            problems.push(ValidationError {
                range: vec![],
                message: format!("The the outputs do not match the expected set of variables. Expected: {:?}, Actual: {:?}", expected_output_vrs, actual_output_vrs),
            });
        }

        // validate continuous state derivatives
        for unknown in &self.derivatives {
            problems.extend(self.validate_unknown(unknown));
            if let Ok(derivative_variable) =
                self.variable_by_value_reference(unknown.valueReference)
                && !matches!(derivative_variable.variability, Variability::Continuous)
            {
                problems.push(ValidationError {
                    range: vec![unknown.range.clone()],
                    message:
                        "Event indicators must be continuous variables of type Float32 or Float64."
                            .to_string(),
                });
            }
        }

        // validate clocked states
        for unknown in &self.clockedStates {
            problems.extend(self.validate_unknown(unknown));

            if let Ok(clocked_state_variable) =
                self.variable_by_value_reference(unknown.valueReference)
            {
                if clocked_state_variable.clocks.is_empty() {
                    problems.push(ValidationError {
                        range: vec![unknown.range.clone()],
                        message: "Clocked states must have at least one clock.".to_string(),
                    });
                }

                if clocked_state_variable.variableType.previous().is_none() {
                    problems.push(ValidationError {
                        range: vec![clocked_state_variable.range.clone(), unknown.range.clone()],
                        message: "Clocked states must have a previous value.".to_string(),
                    });
                }

                if matches!(
                    clocked_state_variable.variableType,
                    VariableType::Clock { .. }
                ) {
                    problems.push(ValidationError {
                        range: vec![unknown.range.clone()],
                        message: "Clocked states must not be clock variables.".to_string(),
                    });
                }
            }
        }

        // validate initial unknowns
        for unknown in &self.initialUnknowns {
            problems.extend(self.validate_unknown(unknown));
        }

        let mut expected_initial_unknown_vrs = HashSet::new();

        for variable in &self.modelVariables {
            if (variable.causality == Causality::Output
                && matches!(
                    variable.initial,
                    Some(Initial::Approx) | Some(Initial::Calculated)
                )
                && variable.clocks.is_empty())
                || variable.causality == Causality::CalculatedParameter
            {
                expected_initial_unknown_vrs.insert(variable.valueReference);
            }
        }

        for derivative in &self.derivatives {
            if let Ok(derivative_variable) =
                self.variable_by_value_reference(derivative.valueReference)
            {
                if matches!(
                    derivative_variable.initial,
                    Some(Initial::Approx) | Some(Initial::Calculated)
                ) {
                    expected_initial_unknown_vrs.insert(derivative_variable.valueReference);
                }

                if let Some(continuous_state_vr) = derivative_variable.variableType.derivative()
                    && let Ok(continuous_state_variable) =
                        self.variable_by_value_reference(continuous_state_vr)
                    && matches!(
                        continuous_state_variable.initial,
                        Some(Initial::Approx) | Some(Initial::Calculated)
                    )
                {
                    expected_initial_unknown_vrs.insert(continuous_state_vr);
                }
            }
        }

        let actual_initial_unknown_vrs = self
            .initialUnknowns
            .iter()
            .map(|u| u.valueReference)
            .collect::<HashSet<u32>>();

        if expected_initial_unknown_vrs != actual_initial_unknown_vrs {
            problems.push(ValidationError {
                range: vec![],
                message: format!("The initial unknowns do not match the expected set of variables. Expected: {:?}, Actual: {:?}", expected_initial_unknown_vrs, actual_initial_unknown_vrs),
            });
        }

        // validate event indicators
        for unknown in self.eventIndicators.iter() {
            problems.extend(self.validate_unknown(unknown));

            if let Ok(event_indicator_variable) =
                self.variable_by_value_reference(unknown.valueReference)
                && !matches!(
                    event_indicator_variable.variableType,
                    VariableType::Float32 { .. } | VariableType::Float64 { .. }
                )
            {
                problems.push(ValidationError {
                    range: vec![unknown.range.clone()],
                    message: "The variable type of the event indicators must be Float32 or Float64"
                        .to_string(),
                });
            }
        }

        problems
    }

    /// validate the value reference, dependencies, and dependencies kind
    fn validate_unknown(&self, unknown: &Unknown) -> Vec<ValidationError> {
        let mut problems = vec![];

        if !self.is_valid_value_reference(unknown.valueReference) {
            problems.push(ValidationError {
                range: vec![unknown.range.clone()],
                message: format!("Illegal value reference: {}", unknown.valueReference),
            });
        }

        if let Some(dependencies) = &unknown.dependencies {
            for dependency_vr in dependencies {
                if !self.is_valid_value_reference(*dependency_vr) {
                    problems.push(ValidationError {
                        range: vec![unknown.range.clone()],
                        message: format!(
                            "Illegal value reference in dependencies: {}",
                            dependency_vr
                        ),
                    });
                }
            }

            if let Some(dependencies_kind) = &unknown.dependenciesKind
                && dependencies.len() != dependencies_kind.len()
            {
                problems.push(ValidationError {
                    range: vec![unknown.range.clone()],
                    message: "The number of elements in dependenciesKind does not match the number of elements in dependencies.".to_string(),
                });
            }
        }

        problems
    }

    fn is_valid_value_reference(&self, valueReference: fmi3ValueReference) -> bool {
        self.modelVariables
            .iter()
            .any(|v| v.valueReference == valueReference)
    }
}
