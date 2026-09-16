use std::{any::TypeId, str::FromStr};

use roxmltree::Node;

use crate::model_description::{
    BaseUnit, Category, DefaultExperiment, DisplayUnit, ModelDescriptionError, Unit,
};

pub(crate) trait NodeExt<'a, 'input> {
    fn get_child(&self, name: &str) -> Option<Node<'a, 'input>>;
    fn get_required_child(&self, name: &str) -> Result<Node<'a, 'input>, ModelDescriptionError>;
    fn get_children(&self, name: &str) -> Vec<Node<'a, 'input>>;
    fn required_attribute(&self, name: &str) -> Result<String, ModelDescriptionError>;
    fn attribute_as<T: FromStr + 'static>(
        &self,
        name: &str,
    ) -> Result<Option<T>, ModelDescriptionError>;
    fn required_attribute_as<T: FromStr + 'static>(
        &self,
        name: &str,
    ) -> Result<T, ModelDescriptionError>;
}

impl<'a, 'input> NodeExt<'a, 'input> for Node<'a, 'input> {
    fn get_child(&self, name: &str) -> Option<Node<'a, 'input>> {
        self.children().find(|n| n.has_tag_name(name))
    }

    fn get_required_child(&self, name: &str) -> Result<Node<'a, 'input>, ModelDescriptionError> {
        self.children()
            .find(|n| n.has_tag_name(name))
            .ok_or_else(|| {
                let message = format!(
                    "Missing required element <{}> in <{}>",
                    name,
                    self.tag_name().name()
                );
                ModelDescriptionError::Parse(message)
            })
    }

    fn get_children(&self, name: &str) -> Vec<Node<'a, 'input>> {
        self.children().filter(|n| n.has_tag_name(name)).collect()
    }

    fn required_attribute(&self, name: &str) -> Result<String, ModelDescriptionError> {
        self.attribute(name)
            .ok_or_else(|| {
                let message = format!("Missing required attribute '{}'", name);
                ModelDescriptionError::Parse(message)
            })
            .map(|s| s.to_string())
    }

    fn attribute_as<T: FromStr + 'static>(
        &self,
        name: &str,
    ) -> Result<Option<T>, ModelDescriptionError> {
        if let Some(literal) = self.attribute(name) {
            let normalized = if TypeId::of::<T>() == TypeId::of::<bool>() {
                match literal {
                    "1" => "true",
                    "0" => "false",
                    _ => literal,
                }
            } else {
                literal
            };
            let result = normalized.parse::<T>().map_err(|_| {
                let message = format!(
                    "Illegal value '{}' for attribute '{}' in <{}>.",
                    literal,
                    name,
                    self.tag_name().name()
                );
                ModelDescriptionError::Parse(message)
            })?;
            Ok(Some(result))
        } else {
            Ok(None)
        }
    }

    fn required_attribute_as<T: FromStr + 'static>(
        &self,
        name: &str,
    ) -> Result<T, ModelDescriptionError> {
        if let Some(value) = self.attribute(name) {
            let normalized = if TypeId::of::<T>() == TypeId::of::<bool>() {
                match value {
                    "1" => "true",
                    "0" => "false",
                    _ => value,
                }
            } else {
                value
            };
            normalized.parse::<T>().map_err(|_| {
                let message = format!(
                    "Illegal value '{}' for attribute '{}' in <{}>.",
                    value,
                    name,
                    self.tag_name().name()
                );
                ModelDescriptionError::Parse(message)
            })
        } else {
            let message = format!(
                "Missing required attribute '{}' in <{}>.",
                name,
                self.tag_name().name()
            );
            Err(ModelDescriptionError::Parse(message))
        }
    }
}

impl BaseUnit {
    pub(crate) fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(BaseUnit {
            kg: node.attribute_as("kg")?.unwrap_or_default(),
            m: node.attribute_as("m")?.unwrap_or_default(),
            s: node.attribute_as("s")?.unwrap_or_default(),
            A: node.attribute_as("A")?.unwrap_or_default(),
            K: node.attribute_as("K")?.unwrap_or_default(),
            mol: node.attribute_as("mol")?.unwrap_or_default(),
            cd: node.attribute_as("cd")?.unwrap_or_default(),
            rad: node.attribute_as("rad")?.unwrap_or_default(),
            factor: node.attribute_as("factor")?.unwrap_or(1.0),
            offset: node.attribute_as("offset")?.unwrap_or_default(),
            range: node.range(),
        })
    }
}

impl DisplayUnit {
    pub(crate) fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(DisplayUnit {
            factor: node.attribute_as("factor")?.unwrap_or(1.0),
            offset: node.attribute_as("offset")?.unwrap_or_default(),
            inverse: node.attribute_as("inverse")?.unwrap_or_default(),
            name: node.required_attribute("name")?,
            range: node.range(),
        })
    }
}

impl Unit {
    pub(crate) fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(Unit {
            name: node.required_attribute("name")?,
            baseUnit: node
                .get_child("BaseUnit")
                .map(|n| BaseUnit::from_node(&n))
                .transpose()?,
            displayUnits: node
                .get_children("DisplayUnit")
                .into_iter()
                .map(|n| DisplayUnit::from_node(&n))
                .collect::<Result<Vec<_>, _>>()?,
            range: node.range(),
        })
    }
}

impl Category {
    pub(crate) fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(Category {
            name: node.required_attribute("name")?,
            description: node.attribute_as("description")?,
        })
    }
}

impl DefaultExperiment {
    pub(crate) fn from_node(node: &roxmltree::Node) -> Result<Self, ModelDescriptionError> {
        Ok(DefaultExperiment {
            startTime: node.attribute_as("startTime")?,
            stopTime: node.attribute_as("stopTime")?,
            tolerance: node.attribute_as("tolerance")?,
            stepSize: node.attribute_as("stepSize")?,
            range: node.range(),
        })
    }
}
