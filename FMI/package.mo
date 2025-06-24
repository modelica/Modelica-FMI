within ;
package FMI "Import Functional Mock-up Units (FMUs) to Modelica"
  extends Modelica.Icons.Package;

  package Menu "FMU Import"

    function importFMU = FMI.Menu.openImportDialog "Import FMU..." annotation(__Dymola_interactive=true, __Dymola_autoExecute=true);

    function openImportDialog

    algorithm

      DymolaCommands.System.Execute("modelica-fmi-gui");

      annotation(__Dymola_interactive=true);

    end openImportDialog;

    annotation (
      __Dymola_toolbar=true,
      __Dymola_interactive=true,
      Protection(hideFromBrowser=true),
      Icon(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,
              100}}), graphics={Bitmap(extent={{-100,-100},{100,100}}, fileName
              ="modelica://FMI/Resources/Images/FMI_package.svg")}));

  end Menu;
annotation (version="0.0.7", uses(Modelica(version="4.0.0")), Icon(graphics={Bitmap(extent={{-80,-60},
            {80,60}}, fileName="modelica://FMI/Resources/Images/FMI_bare.svg")}),
    Documentation(info="<html>

<p>A Modelica package to import <a href=\"https://fmi-standard.org/\">Functional Mock-up Units</a> (FMUs).</p>

<h5>Install the modelica-fmi command</h5>

<p>To import FMUs the library requires the <code>modelica-fmi</code> and <code>modelica-fmi-gui</code>commands.</p>

<p>After <a href=\"https://docs.astral.sh/uv/getting-started/installation/\">installing uv</a> run

<pre>uv tool install --reinstall https://github.com/modelica/modelica-fmi/releases/download/v0.0.7/modelica_fmi-0.0.7-py3-none-any.whl</pre>

to install the <code>modelica-fmi</code> command.</p>

<h5>Import an FMU</h5>

<p>To import an FMU into a Modelica package using the import dialog run <code>modelica-fmi-gui</code>.</p>

<p>In Dymola you can select <code>Tools > FMU Import > Import FMU...</code>.</p>

<p>To import an FMU on the command line run <code>modelica-fmi --help</code>.</p>

</html>"),
    __Dymola_Commands(executeCall(description=
            "Import an FMU to an existing package") = Execute(
        "modelica-fmi-gui") "Import FMU..."), __Dymola_containsMenu=true);
end FMI;
