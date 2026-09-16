use crate::{
    model_description::fmi2::ModelDescription,
    sim::{
        SimulationError,
        fmi2::{Trajectories, parse_variable_value},
    },
};
use std::{io::Read, path::Path, sync::Arc};

pub fn write_csv<P: AsRef<Path>>(
    trajectories: &Trajectories,
    output_file: P,
) -> std::io::Result<()> {
    let mut writer = csv::Writer::from_path(output_file)?;

    let mut header = vec!["time".to_string()];

    for variable in trajectories.variables() {
        header.push(variable.name.clone());
    }

    writer.write_record(&header)?;

    for (time, row) in trajectories.time.iter().zip(trajectories.rows.iter()) {
        let values = row.iter().map(|value| value.to_literal());
        let record: Vec<String> = std::iter::once(time.to_string()).chain(values).collect();
        writer.write_record(&record)?;
    }

    writer.flush()?;

    Ok(())
}

pub fn read_csv<R: Read>(
    reader: R,
    model_description: Arc<ModelDescription>,
) -> Result<Trajectories, SimulationError> {
    let mut reader = csv::Reader::from_reader(reader);

    let headers = match reader.headers() {
        Ok(record) => record,
        Err(e) => {
            return Err(SimulationError::Parse(format!(
                "Failed to read headers. {e}"
            )));
        }
    };

    let mut variable_indices = vec![];
    let mut variable_types = vec![];

    for name in headers.iter().skip(1) {
        variable_indices.push(model_description.variable_index_by_name(name)?);
        variable_types.push(&model_description.variable_by_name(name)?.variableType);
    }

    let mut time = vec![];
    let mut rows = vec![];

    for (i, result) in reader.records().enumerate() {
        match result {
            Ok(record) => {
                let mut row = vec![];
                let mut it = record.iter();

                let next_time: f64 = it
                    .next()
                    .ok_or_else(|| {
                        SimulationError::Parse(format!(
                            "Missing time value in row {}.",
                            i.saturating_add(2)
                        ))
                    })?
                    .parse()
                    .map_err(|e| {
                        SimulationError::Parse(format!(
                            "Failed to parse time value '{}' in row {}: {}",
                            record.get(0).unwrap_or(""),
                            i.saturating_add(2),
                            e
                        ))
                    })?;

                time.push(next_time);

                for (j, (literal, variable_type)) in it.zip(variable_types.iter()).enumerate() {
                    row.push(parse_variable_value(variable_type, literal).map_err(|e| {
                        SimulationError::Parse(format!(
                            "Failed to parse '{literal:?}' (row {}, column {}): {e}",
                            i.saturating_add(2),
                            j.saturating_add(2)
                        ))
                    })?);
                }

                rows.push(row);
            }
            Err(e) => {
                return Err(SimulationError::Parse(format!("Error reading input. {e}")));
            }
        }
    }

    let trajectories = Trajectories {
        model_description,
        variable_indices,
        time,
        rows,
    };

    trajectories.validate().map_err(SimulationError::Parse)?;

    Ok(trajectories)
}
