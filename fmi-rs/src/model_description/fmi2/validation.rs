use std::collections::{HashMap, HashSet};

use crate::model_description::ValidationError;
use crate::model_description::fmi2::{
    Causality, Initial, ModelDescription, ScalarVariable, Unknown, Variability,
    VariableNamingConvention, VariableType,
};
use crate::model_description::validation::validate_structured_variable_name;

impl ModelDescription {
    /// Checks if the model description is valid according to the FMI 2.0 specification.
    /// Returns a list of problems found in the model description.
    /// If the list is empty, the model description is valid.
    pub fn validate(&self) -> Vec<ValidationError> {
        let mut problems = vec![];

        let mut independent_variable: Option<&ScalarVariable> = None;

        let mut variable_names: HashMap<&String, &ScalarVariable> = HashMap::new();

        if self.modelExchange.is_none() && self.coSimulation.is_none() {
            problems.push(ValidationError {
                range: vec![],
                message: "No interface type defined.".to_string(),
            });
        }

        for variable in &self.modelVariables {
            // check variable name
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

            // check for duplicate variable names
            if let Some(duplicate) = variable_names.get(&variable.name) {
                problems.push(ValidationError {
                    range: vec![duplicate.range.clone(), variable.range.clone()],
                    message: format!("Duplicate variable name: '{}'", variable.name),
                });
            } else {
                variable_names.insert(&variable.name, variable);
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

                if !matches!(variable.variableType, VariableType::Real { .. }) {
                    problems.push(ValidationError {
                        range: vec![variable.range.clone()],
                        message: "The independent variable must be a Real variable.".to_string(),
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

            // check variability
            if !matches!(variable.variableType, VariableType::Real { .. })
                && variable.variability == Variability::Continuous
            {
                problems.push(ValidationError {
                    range: vec![variable.range.clone()],
                    message: "Only Real variables can be continuous.".to_string(),
                });
            }

            // check combination of causality and variability, and initial
            match (
                &variable.causality,
                &variable.variability,
                &variable.initial,
            ) {
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

        let expected_output_indices = self
            .modelVariables
            .iter()
            .enumerate()
            .filter(|i| i.1.causality == Causality::Output)
            .map(|i| (i.0.saturating_add(1)) as u32)
            .collect::<HashSet<u32>>();

        let actual_output_indices = self
            .outputs
            .iter()
            .map(|u| u.index)
            .collect::<HashSet<u32>>();

        if expected_output_indices != actual_output_indices {
            problems.push(ValidationError {
                range: vec![],
                message: format!("The the outputs do not match the expected set of variables. Expected: {:?}, Actual: {:?}", expected_output_indices, actual_output_indices),
            });
        }

        // validate continuous state derivatives
        for unknown in &self.derivatives {
            problems.extend(self.validate_unknown(unknown));
        }

        for unknown in &self.derivatives {
            let derivative_variable = match self.variable_by_index(unknown.index) {
                Ok(variable) => variable,
                Err(_) => {
                    problems.push(ValidationError {
                        range: vec![unknown.range.clone()],
                        message: format!("Illegal variable index: {}", unknown.index),
                    });
                    continue;
                }
            };

            if let VariableType::Real { derivative, .. } = &derivative_variable.variableType {
                if let Some(derivative_index) = derivative {
                    match self.variable_by_index(*derivative_index) {
                        Ok(state_variable) => {
                            if !matches!(state_variable.variableType, VariableType::Real { .. }) {
                                problems.push(ValidationError {
                                    range: vec![derivative_variable.range.clone()],
                                    message: format!("The continuous state variable {} referenced by the derivative {} is not a Real variable", state_variable.name, derivative_variable.name),
                                });
                            }
                        }
                        Err(_) => {
                            problems.push(ValidationError {
                                range: vec![derivative_variable.range.clone()],
                                message: format!("Attribute derivative of variable {} is not a valid variable index", derivative_variable.name),
                            });
                            continue;
                        }
                    };
                } else {
                    problems.push(ValidationError {
                        range: vec![derivative_variable.range.clone()],
                        message: format!(
                            "Variable {} is not a derivative",
                            derivative_variable.name
                        ),
                    });
                }
            } else {
                problems.push(ValidationError {
                    range: vec![derivative_variable.range.clone()],
                    message: format!(
                        "Variable {} is not a real variable",
                        derivative_variable.name
                    ),
                });
            }
        }

        // validate initial unknowns
        for unknown in &self.initialUnknowns {
            problems.extend(self.validate_unknown(unknown));
        }

        let mut expected_initial_unknown_indices = self
            .modelVariables
            .iter()
            .enumerate()
            .filter(|(_index, variable)| {
                (variable.causality == Causality::Output
                    && matches!(
                        variable.initial,
                        Some(Initial::Approx) | Some(Initial::Calculated)
                    ))
                    || variable.causality == Causality::CalculatedParameter
            })
            .map(|(index, _variable)| (index.saturating_add(1)) as u32)
            .collect::<HashSet<u32>>();

        for derivative in &self.derivatives {
            if let Ok(derivative_variable) = self.variable_by_index(derivative.index) {
                if matches!(
                    derivative_variable.initial,
                    Some(Initial::Approx) | Some(Initial::Calculated)
                ) {
                    expected_initial_unknown_indices.insert(derivative.index);
                }

                if let Ok(continuous_state_index) = derivative_variable.variableType.derivative()
                    && let Ok(continuous_state_variable) =
                        self.variable_by_index(continuous_state_index)
                    && matches!(
                        continuous_state_variable.initial,
                        Some(Initial::Approx) | Some(Initial::Calculated)
                    )
                {
                    expected_initial_unknown_indices.insert(continuous_state_index);
                }
            }
        }

        let actual_initial_unknown_indices = self
            .initialUnknowns
            .iter()
            .map(|u| u.index)
            .collect::<HashSet<u32>>();

        if expected_initial_unknown_indices != actual_initial_unknown_indices {
            problems.push(ValidationError {
                range: vec![],
                message: format!("The initial unknowns do not match the expected set of variables. Expected: {:?}, Actual: {:?}", expected_initial_unknown_indices, actual_initial_unknown_indices),
            });
        }

        problems
    }

    fn validate_unknown(&self, unknown: &Unknown) -> Vec<ValidationError> {
        let mut problems = vec![];

        if !self.is_valid_variable_index(unknown.index) {
            problems.push(ValidationError {
                range: vec![],
                message: format!("Illegal variable index: {}", unknown.index),
            });
        }

        if let Some(dependencies) = &unknown.dependencies {
            for dependency_index in dependencies {
                if !self.is_valid_variable_index(*dependency_index) {
                    problems.push(ValidationError {
                        range: vec![unknown.range.clone()],
                        message: format!(
                            "Illegal variable index in dependencies: {}",
                            dependency_index
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

    fn is_valid_variable_index(&self, index: u32) -> bool {
        index > 0 && index <= self.modelVariables.len() as u32
    }
}
