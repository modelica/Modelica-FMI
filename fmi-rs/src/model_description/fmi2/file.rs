use crate::model_description::file::NodeExt;
use crate::model_description::{Category, ModelDescriptionError};
use crate::model_description::{Unit, fmi2::SimpleType};
use roxmltree::Node;
use std::path::Path;
use std::str::FromStr;
use std::vec;

use crate::model_description::fmi2::{
    Causality, CoSimulation, DefaultExperiment, DependencyKind, Initial, Item, ModelDescription,
    ModelExchange, ScalarVariable, Unknown, Variability, VariableNamingConvention, VariableType,
};

impl ModelExchange {
    fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        let sourceFiles = node
            .get_child("SourceFiles")
            .map(|n| n.get_children("File"))
            .into_iter()
            .flatten()
            .map(|n| n.required_attribute("name"))
            .collect::<Result<Vec<_>, _>>()?;

        Ok(ModelExchange {
            sourceFiles,
            modelIdentifier: node.required_attribute_as("modelIdentifier")?,
            needsExecutionTool: node.attribute_as("needsExecutionTool")?.unwrap_or_default(),
            completedIntegratorStepNotNeeded: node
                .attribute_as("completedIntegratorStepNotNeeded")?
                .unwrap_or_default(),
            canBeInstantiatedOnlyOncePerProcess: node
                .attribute_as("canBeInstantiatedOnlyOncePerProcess")?
                .unwrap_or_default(),
            canNotUseMemoryManagementFunctions: node
                .attribute_as("canNotUseMemoryManagementFunctions")?
                .unwrap_or_default(),
            canGetAndSetFMUstate: node
                .attribute_as("canGetAndSetFMUstate")?
                .unwrap_or_default(),
            canSerializeFMUstate: node
                .attribute_as("canSerializeFMUstate")?
                .unwrap_or_default(),
            providesDirectionalDerivative: node
                .attribute_as("providesDirectionalDerivative")?
                .unwrap_or_default(),
            range: node.range(),
        })
    }
}

impl CoSimulation {
    fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        let sourceFiles = node
            .get_child("SourceFiles")
            .map(|n| n.get_children("File"))
            .into_iter()
            .flatten()
            .map(|n| n.required_attribute("name"))
            .collect::<Result<Vec<_>, _>>()?;

        Ok(CoSimulation {
            sourceFiles,
            modelIdentifier: node.required_attribute_as("modelIdentifier")?,
            needsExecutionTool: node.attribute_as("needsExecutionTool")?.unwrap_or_default(),
            canHandleVariableCommunicationStepSize: node
                .attribute_as("canHandleVariableCommunicationStepSize")?
                .unwrap_or_default(),
            canInterpolateInputs: node
                .attribute_as("canInterpolateInputs")?
                .unwrap_or_default(),
            maxOutputDerivativeOrder: node
                .attribute_as("maxOutputDerivativeOrder")?
                .unwrap_or_default(),
            canRunAsynchronuously: node
                .attribute_as("canRunAsynchronuously")?
                .unwrap_or_default(),
            canBeInstantiatedOnlyOncePerProcess: node
                .attribute_as("canBeInstantiatedOnlyOncePerProcess")?
                .unwrap_or_default(),
            canNotUseMemoryManagementFunctions: node
                .attribute_as("canNotUseMemoryManagementFunctions")?
                .unwrap_or_default(),
            canGetAndSetFMUstate: node
                .attribute_as("canGetAndSetFMUstate")?
                .unwrap_or_default(),
            canSerializeFMUstate: node
                .attribute_as("canSerializeFMUstate")?
                .unwrap_or_default(),
            providesDirectionalDerivative: node
                .attribute_as("providesDirectionalDerivative")?
                .unwrap_or_default(),
            range: node.range(),
        })
    }
}

impl ModelDescription {
    pub fn from_path(path: &Path) -> Result<ModelDescription, ModelDescriptionError> {
        let text = std::fs::read_to_string(path)?;
        Self::from_string(&text)
    }

    pub fn from_string(text: &str) -> Result<ModelDescription, ModelDescriptionError> {
        let opt = roxmltree::ParsingOptions {
            allow_dtd: true,
            ..roxmltree::ParsingOptions::default()
        };

        let doc = roxmltree::Document::parse_with_options(text, opt)
            .map_err(|e| ModelDescriptionError::Parse(e.to_string()))?;

        Self::from_node(&doc.root_element())
            .map_err(|e| ModelDescriptionError::Parse(format!("{e}")))
    }

    pub fn from_node(root: &Node) -> Result<ModelDescription, ModelDescriptionError> {
        let fmi_version = root.required_attribute("fmiVersion")?;

        if fmi_version != "2.0" {
            let message = format!("Expected FMI version 2.0, but was {fmi_version}");
            return Err(ModelDescriptionError::Parse(message));
        }

        let mut modelVariables = vec![];

        for child in root
            .get_required_child("ModelVariables")?
            .get_children("ScalarVariable")
        {
            let name = child.required_attribute("name")?;

            let valueReference = child.required_attribute_as("valueReference")?;

            let description = child.attribute_as("description")?;

            let canHandleMultipleSetPerTimeInstant = child
                .attribute_as("canHandleMultipleSetPerTimeInstant")?
                .unwrap_or_default();

            let variableType = Self::get_variable_type(&child)?;

            let causality = child
                .attribute("causality")
                .map(Causality::from_str)
                .transpose()?
                .unwrap_or(Causality::Local);

            let variability = child
                .attribute("variability")
                .map(Variability::from_str)
                .transpose()?
                .unwrap_or(Variability::Continuous);

            let mut initial = child
                .attribute("initial")
                .map(Initial::from_str)
                .transpose()?;

            if initial.is_none() && causality != Causality::Independent {
                initial = match (&variability, &causality) {
                    (Variability::Constant, Causality::Output) => Some(Initial::Exact),
                    (Variability::Constant, Causality::Local) => Some(Initial::Exact),
                    (Variability::Fixed, Causality::Parameter) => Some(Initial::Exact),
                    (Variability::Fixed, Causality::CalculatedParameter) => {
                        Some(Initial::Calculated)
                    }
                    (Variability::Fixed, Causality::Local) => Some(Initial::Calculated),
                    (Variability::Tunable, Causality::Parameter) => Some(Initial::Exact),
                    (Variability::Tunable, Causality::CalculatedParameter) => {
                        Some(Initial::Calculated)
                    }
                    (Variability::Tunable, Causality::Local) => Some(Initial::Calculated),
                    (Variability::Discrete, Causality::Input) => Some(Initial::Exact),
                    (Variability::Discrete, Causality::Output) => Some(Initial::Calculated),
                    (Variability::Discrete, Causality::Local) => Some(Initial::Calculated),
                    (Variability::Continuous, Causality::Input) => Some(Initial::Exact),
                    (Variability::Continuous, Causality::Output) => Some(Initial::Calculated),
                    (Variability::Continuous, Causality::Local) => Some(Initial::Calculated),
                    _ => {
                        let message = format!(
                            "Illegal combination of variability and causality: {:?} {:?}",
                            variability, causality
                        );
                        return Err(ModelDescriptionError::Parse(message));
                    }
                };
            }

            let variable = ScalarVariable {
                variableType,
                name,
                valueReference,
                description,
                causality,
                variability,
                initial,
                canHandleMultipleSetPerTimeInstant,
                range: child.range(),
            };

            modelVariables.push(variable);
        }

        let logCategories = root
            .get_child("LogCategories")
            .map(|n| n.get_children("Category"))
            .into_iter()
            .flatten()
            .map(|n| Category::from_node(&n))
            .collect::<Result<Vec<_>, _>>()?;

        let defaultExperiment = root
            .get_child("DefaultExperiment")
            .map(|n| DefaultExperiment::from_node(&n))
            .transpose()?;

        let coSimulation = root
            .get_child("CoSimulation")
            .map(|n| CoSimulation::from_node(&n))
            .transpose()?;

        let modelExchange = root
            .get_child("ModelExchange")
            .map(|n| ModelExchange::from_node(&n))
            .transpose()?;

        let unitDefintions = root
            .get_child("UnitDefintions")
            .map(|n| n.get_children("Unit"))
            .into_iter()
            .flatten()
            .map(|n| Unit::from_node(&n))
            .collect::<Result<Vec<_>, _>>()?;

        let typeDefinitions = root
            .get_child("TypeDefinitions")
            .map(|n| n.get_child("SimpleType"))
            .into_iter()
            .flatten()
            .map(|n| SimpleType::from_node(&n))
            .collect::<Result<Vec<_>, _>>()?;

        let model_description = ModelDescription {
            modelName: root.required_attribute("modelName")?,
            guid: root.required_attribute("guid")?,
            description: root.attribute_as("description")?,
            author: root.attribute_as("author")?,
            version: root.attribute_as("version")?,
            copyright: root.attribute_as("copyright")?,
            license: root.attribute_as("license")?,
            generationTool: root.attribute_as("generationTool")?,
            generationDateAndTime: root.attribute_as("generationDateAndTime")?,
            variableNamingConvention: root
                .attribute("variableNamingConvention")
                .map(|n| n.parse())
                .transpose()?
                .unwrap_or(VariableNamingConvention::Flat),
            logCategories,
            defaultExperiment,
            coSimulation,
            modelExchange,
            unitDefintions,
            typeDefinitions,
            modelVariables,
            numberOfEventIndicators: root
                .attribute_as("numberOfEventIndicators")?
                .unwrap_or_default(),
            outputs: Self::get_unkonwns(root, "Outputs")?,
            derivatives: Self::get_unkonwns(root, "Derivatives")?,
            initialUnknowns: Self::get_unkonwns(root, "InitialUnknowns")?,
        };

        Ok(model_description)
    }

    fn get_variable_type(node: &Node) -> Result<VariableType, ModelDescriptionError> {
        for child in node.children() {
            if child.has_tag_name("Real") {
                return Ok(VariableType::Real {
                    declaredType: child.attribute_as("declaredType")?,
                    quantity: child.attribute_as("quantity")?,
                    unit: child.attribute_as("unit")?,
                    displayUnit: child.attribute_as("displayUnit")?,
                    relativeQuantity: child
                        .attribute("relativeQuantity")
                        .map(|s| s == "true")
                        .unwrap_or(false),
                    min: child.attribute_as("min")?,
                    max: child.attribute_as("max")?,
                    nominal: child.attribute_as("nominal")?,
                    unbounded: child
                        .attribute("unbounded")
                        .map(|s| s == "true")
                        .unwrap_or(false),
                    start: child.attribute_as("start")?,
                    derivative: child.attribute_as("derivative")?,
                    reinit: child.attribute_as("reinit")?.unwrap_or_default(),
                });
            } else if child.has_tag_name("Integer") {
                return Ok(VariableType::Integer {
                    declaredType: child.attribute_as("declaredType")?,
                    quantity: child.attribute_as("quantity")?,
                    min: child.attribute_as("min")?,
                    max: child.attribute_as("max")?,
                    start: child.attribute_as("start")?,
                });
            } else if child.has_tag_name("Boolean") {
                return Ok(VariableType::Boolean {
                    declaredType: child.attribute_as("declaredType")?,
                    start: child.attribute_as("start")?,
                });
            } else if child.has_tag_name("String") {
                return Ok(VariableType::String {
                    declaredType: child.attribute_as("declaredType")?,
                    start: child.attribute_as("start")?,
                });
            } else if child.has_tag_name("Enumeration") {
                return Ok(VariableType::Enumeration {
                    declaredType: child.required_attribute("declaredType")?,
                    quantity: child.attribute_as("quantity")?,
                    min: child.attribute_as("min")?,
                    max: child.attribute_as("max")?,
                    start: child.attribute_as("start")?,
                });
            }
        }

        Err(ModelDescriptionError::Parse(
            "Missing variable type element".to_string(),
        ))
    }

    fn get_unkonwns(root: &Node, name: &str) -> Result<Vec<Unknown>, ModelDescriptionError> {
        let modelStructure = if let Some(modelStructure) = root.get_child("ModelStructure") {
            modelStructure
        } else {
            return Ok(Vec::new());
        };

        let container = if let Some(container) = modelStructure.get_child(name) {
            container
        } else {
            return Ok(Vec::new());
        };

        let mut unkonwns = vec![];

        for child in container.get_children("Unknown") {
            let index = child.required_attribute("index")?.parse().map_err(|e| {
                let message = format!("{e}");
                ModelDescriptionError::Parse(message)
            })?;

            let dependencies: Option<Vec<u32>> = match child.attribute("dependencies") {
                Some(dependencies) => Some(
                    dependencies
                        .split_whitespace()
                        .map(|s| s.parse::<u32>())
                        .collect::<Result<Vec<_>, _>>()
                        .map_err(|e| {
                            let message = format!("{e}");
                            ModelDescriptionError::Parse(message)
                        })?,
                ),
                None => None,
            };

            let dependenciesKind: Option<Vec<DependencyKind>> =
                match child.attribute("dependenciesKind") {
                    Some(dependenciesKind) => Some(
                        dependenciesKind
                            .split_whitespace()
                            .map(DependencyKind::from_str)
                            .collect::<Result<Vec<_>, _>>()
                            .map_err(|e| ModelDescriptionError::Parse(e.to_string()))?,
                    ),
                    None => None,
                };

            unkonwns.push(Unknown {
                index,
                dependencies,
                dependenciesKind,
                range: child.range(),
            });
        }

        Ok(unkonwns)
    }
}

impl SimpleType {
    fn from_node(node: &Node) -> Result<Self, ModelDescriptionError> {
        let name = node.required_attribute("name")?;
        let description = node.attribute_as("description")?;

        for child in node.children() {
            if child.has_tag_name("Real") {
                return Ok(SimpleType::Real {
                    name,
                    description,
                    quantity: child.attribute_as("quantity")?,
                    unit: child.attribute_as("unit")?,
                    displayUnit: child.attribute_as("displayUnit")?,
                    relativeQuantity: child.attribute_as("relativeQuantity")?.unwrap_or_default(),
                    unbounded: child.attribute_as("unbounded")?.unwrap_or_default(),
                    min: child.attribute_as("min")?,
                    max: child.attribute_as("max")?,
                    nominal: child.attribute_as("nominal")?,
                    range: node.range(),
                });
            } else if child.has_tag_name("Integer") {
                return Ok(SimpleType::Integer {
                    name,
                    description,
                    quantity: child.attribute_as("quantity")?,
                    min: child.attribute_as("min")?,
                    max: child.attribute_as("max")?,
                    range: node.range(),
                });
            } else if child.has_tag_name("Boolean") {
                return Ok(SimpleType::Boolean {
                    name,
                    description,
                    range: node.range(),
                });
            } else if child.has_tag_name("String") {
                return Ok(SimpleType::String {
                    name,
                    description,
                    range: node.range(),
                });
            } else if child.has_tag_name("Enumeration") {
                let mut items = vec![];
                for grand_child in child.children() {
                    if grand_child.has_tag_name("Item") {
                        items.push(Item::from_node(&grand_child)?);
                    }
                }
                return Ok(SimpleType::Enumeration {
                    name,
                    description,
                    items,
                    quantity: child.attribute_as("quantity")?,
                    range: node.range(),
                });
            }
        }

        Err(ModelDescriptionError::Parse(
            "Missing variable type element".to_string(),
        ))
    }
}

impl Item {
    fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(Item {
            name: node.required_attribute("name")?,
            description: node.attribute_as("description")?,
            value: node.required_attribute_as("value")?,
            range: node.range(),
        })
    }
}
