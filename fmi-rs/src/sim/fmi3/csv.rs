use crate::{
    model_description::fmi3::ModelDescription,
    sim::{
        SimulationError, SimulationSliceExt,
        fmi3::{Trajectories, parse_variable_value},
    },
};
use std::{io::Read, iter::once, path::Path, sync::Arc};

pub fn write_csv<P: AsRef<Path>>(
    trajectories: &Trajectories,
    output_file: P,
) -> std::io::Result<()> {
    let mut writer = csv::Writer::from_path(output_file)?;

    let mut header = vec!["time".to_string()];

    for variable_index in trajectories.variable_indices.iter() {
        if let Some(variable) = &trajectories
            .model_description
            .modelVariables
            .get(*variable_index)
        {
            header.push(variable.name.clone());
        }
    }

    writer.write_record(&header)?;

    for (time, row) in trajectories.time.iter().zip(trajectories.rows.iter()) {
        let record = once(time.to_string()).chain(row.iter().map(|v| v.to_literal()));
        writer.write_record(record)?;
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
                "Failed to read headers: {e}"
            )));
        }
    };

    let variable_indices: Vec<usize> = headers
        .iter()
        .skip(1)
        .map(|name| model_description.variable_index_by_name(name))
        .collect::<Result<Vec<_>, _>>()?;

    let mut time = vec![];
    let mut rows = vec![];

    for (i, result) in reader.records().enumerate() {
        let record = result.map_err(|e| SimulationError::Parse(e.to_string()))?;

        let mut row = vec![];
        let mut it = record.iter();

        let next_time: f64 = it
            .next()
            .ok_or_else(|| {
                SimulationError::Parameter(format!(
                    "Missing time value in row {}",
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

        for (j, literal) in it.enumerate() {
            let variable_index = variable_indices.try_get(j)?;
            let variable = &model_description.modelVariables.try_get(*variable_index)?;
            row.push(
                parse_variable_value(&variable.variableType, literal).map_err(|e| {
                    SimulationError::Parse(format!(
                        "Failed to parse '{literal:?}' (row {}, column {}): {e}",
                        i.saturating_add(2),
                        j.saturating_add(2)
                    ))
                })?,
            );
        }

        rows.push(row);
    }

    let trajectories = Trajectories {
        model_description,
        time,
        variable_indices,
        rows,
    };

    trajectories.validate().map_err(SimulationError::Parse)?;

    Ok(trajectories)
}
