within ;
package FMI "Import Functional Mock-up Units (FMUs) to Modelica"
  extends Modelica.Icons.Package;

annotation (version="0.0.3", uses(Modelica(version="4.0.0")), Icon(graphics={Bitmap(extent={{-80,-60},
            {80,60}},        fileName="modelica://FMI/Resources/FMI_bare.png")}),
    Documentation(info="<html>

<p>A Modelica package to import <a href=\"https://fmi-standard.org/\">Functional Mock-up Units</a> (FMUs).</p>

<h5>Install the modelica-fmi command</h5>

<p>To import FMUs the library requires the <code>modelica-fmi</code> command.</p>

<p>After <a href=\"https://docs.astral.sh/uv/getting-started/installation/\">installing uv</a> run

<pre>uv tool install --reinstall https://github.com/modelica/modelica-fmi/releases/download/v0.0.3/modelica_fmi-0.0.3-py3-none-any.whl</pre>

to install the <code>modelica-fmi</code> command.</p>

<h5>Import an FMU</h5>

<p>To import an FMU into a Modelica package run <a href=\"modelica://FMI.importFMU\">FMI.importFMU()</a>.</p>

</html>"));
end FMI;
