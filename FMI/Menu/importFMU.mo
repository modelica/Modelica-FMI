within FMI.Menu;
function importFMU

  input String fmuPath = "C:/Users/tsr2/Documents/Dymola/Feedthrough.fmu" "Path to the FMU to import" annotation(Dialog(loadSelector(filter="FMU files (*.fmu)", caption="Select FMU file")));
  input String modelPath = "C:/Users/tsr2/Documents/Dymola/Feeder.mo" "Path to the Modelica file to create" annotation(Dialog(saveSelector(filter="Modelica files (*.mo)", caption="New Modelica file")));
  input Boolean overwrite = true "Overwrite existing files" annotation (choices(checkBox=true));
  output String fullOutput = "";
  output Boolean success;

protected

  String tempFile = "cmd_out.txt";
  String command;
  String lines[:];

algorithm

  command := "\"" + Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/Library/win64/modelica-fmi.exe") + "\"";

  if overwrite then
    command := command + " --overwrite";
  end if;

  command := command + " \"" + fmuPath + "\" \"" + modelPath + "\" > \"" + tempFile + "\"";

  Modelica.Utilities.Streams.print("Executing command: " + command);

  success := Modelica.Utilities.System.command(command) == 0;

  lines := Modelica.Utilities.Streams.readFile(tempFile);

  for i in 1:size(lines, 1) loop
    fullOutput := fullOutput + lines[i] + "\n";
  end for;

  Modelica.Utilities.Streams.print(fullOutput);

  annotation(__Dymola_interactive=true);

end importFMU;
