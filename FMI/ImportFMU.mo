within FMI;
function ImportFMU
  extends Modelica.Icons.Function;

  input String filename annotation(Dialog(loadSelector(filter="Functional Mock-up Units (*.fmu)", caption="Select FMU to import")));
  input String packageFile annotation(Dialog(saveSelector(filter="Modelica File (*.mo)", caption="Modelica file to generate")));
  input String interfaceType = "CoSimulation" annotation(Dialog(choices(choice="CoSimulation" "Co-Simulation", choice="ModelExchange" "Model Exchange")));;
  output Boolean success;

algorithm

  success := Modelica.Utilities.System.command("python -m modelica_fmi.import_fmu_to_modelica \"" + filename + "\" \"" + packageFile + "\" " + interfaceType + " > import_fmu_to_modelica.txt") == 0;

end ImportFMU;
