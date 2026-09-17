within ;
package FMI "Use Functional Mock-up Units in Modelica"
  extends Modelica.Icons.Package;

annotation (version="0.0.9", Icon(graphics={Bitmap(extent={{-80,-60},
            {80,60}}, fileName="modelica://FMI/Resources/Images/FMI_bare.svg")}),
    Documentation(info="<html>

<h5>Import an FMU</h5>

<p>To import an FMU run <code>modelica-fmi</code> from the respective platform directory (e.g. <code>FMI/Resources/Library/win64</code> on Windows).</p>

<p>
<pre>
modelica-fmi /path/to/BouncingBall.fmu /path/to/BouncingBall.mo
</pre>
</p>

</html>"));
end FMI;
