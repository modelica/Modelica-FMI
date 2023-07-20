within FMI;
function ImportFMU

  input String filename annotation(Dialog(__Dymola_loadSelector(filter="Functional Mock-up Units (*.fmu)", caption="Select FMU to import")));
  input String packageFile annotation(Dialog(__Dymola_saveSelector(filter="Modelica File (*.mo)", caption="Modelica file to generate")));
  input String interfaceType = "CoSimulation";
  output Boolean success;

algorithm

  success := Modelica.Utilities.System.command("python -m modelica_fmi.import_fmu_to_modelica \"" + filename + "\" \"" + packageFile + "\" " + interfaceType + " > import_fmu_to_modelica.txt") == 0;

end ImportFMU;
