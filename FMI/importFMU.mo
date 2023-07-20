within FMI;
function importFMU "Import an FMU into a Modelica library"
  extends Modelica.Icons.Function;

  input String FMUPath "Path of the FMU to import" annotation(Dialog(loadSelector(filter="Functional Mock-up Units (*.fmu)", caption="Select FMU to import")));
  input String modelPath "Path of the Modelica file to generate" annotation(Dialog(saveSelector(filter="Modelica File (*.mo)", caption="Modelica file to generate")));
  output Boolean success;

algorithm

  success := Modelica.Utilities.System.command("modelica-fmi \"" + FMUPath + "\" \"" + modelPath + "\" > import_fmu_to_modelica.txt") == 0;

  annotation (Documentation(info="<html>

<h4>Example</h4>

<pre>
FMI.importFMU(
  FMUPath=\"C:/work/Rectifier.fmu\",
  modelPath=\"C:/work/MyLibrary/Rectifier.mo\"
);
</pre>

<p><code>modelPath</code> is the <code>*.mo</code> file to be created and must be located inside the directory structure of an existing Modelica package.</p>

<p>This Modelica package has to be reloaded after the import.</p>

<p><code>importFMU()</code> is calling the <code>modelica-fmi</code> command that can also be used directly e.g. to get error messages in case <code>importFMU()</code> returns <code>false</code>.</p>

<pre>
$ modelica-fmi --help
usage: modelica-fmi [-h] [-v] fmu_path model_path

Import an FMU into a Modelica library

positional arguments:
  fmu_path       Path of the FMU to import
  model_path     Path of the Modelica file to generate

options:
  -h, --help     show this help message and exit
  -v, --version  print the program version
</pre>

</html>"));
end importFMU;
