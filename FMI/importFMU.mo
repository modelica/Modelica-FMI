within FMI;
function importFMU "Import an FMU into a Modelica library"
  extends Modelica.Icons.Function;

  input String FMUPath "Path of the FMU to import" annotation(Dialog(loadSelector(filter="Functional Mock-up Units (*.fmu)", caption="Select FMU to import")));
  input String modelPath "Path of the Modelica file to generate" annotation(Dialog(saveSelector(filter="Modelica File (*.mo)", caption="Modelica file to generate")));
  output Boolean success;

algorithm

  success := Modelica.Utilities.System.command("modelica-fmi \"" + FMUPath + "\" \"" + modelPath + "\" > import_fmu_to_modelica.txt") == 0;

end importFMU;
