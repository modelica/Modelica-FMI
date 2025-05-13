within @=package=@;

model @=modelName=@
@@ if description @@
  "@=description=@"
@@ endif @@

@@ block imports @@
@@ endblock @@

  parameter Modelica.Units.SI.Time stopTime = Modelica.Constants.inf annotation(Dialog(tab="FMI", group="Parameters"));

  parameter Real tolerance = 0.0 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter Boolean visible = false annotation(Dialog(tab="FMI", group="Parameters"));

  parameter Boolean loggingOn = false annotation(Dialog(tab="FMI", group="Parameters"));

  parameter Boolean logToFile = false annotation(Dialog(tab="FMI", group="Parameters"));

  parameter String logFile = getInstanceName() + ".txt" annotation(Dialog(tab="FMI", group="Parameters"));

  parameter Boolean logFMICalls = false annotation(Dialog(tab="FMI", group="Parameters"));
@@ block parameters @@
@@ endblock @@
@@ for variable in parameters @@

  parameter @=fmi_type(variable, prefix=True)=@ @=name(variable)=@@@ if variable.dimensions @@@=subscripts(variable)=@@@ endif @@@=modifiers(variable)=@ = @=start_value(variable)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@@=choices(variable)=@;
@@ endfor @@
@@ for variable in inputs @@

  parameter @=fmi_type(variable, prefix=True)=@ @=name(variable, '_start')=@@=subscripts(variable)=@@=modifiers(variable)=@ = @=start_value(variable)=@;
@@ endfor @@
@@ for variable in inputs @@

  @=fmi_type(variable, prefix=True)=@Input @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable, start=True)=@ @=annotations[variable.name]=@;
@@ endfor @@
@@ for variable in outputs @@

  @=fmi_type(variable, prefix=True)=@Output @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable)=@ @=annotations[variable.name]=@;
@@ endfor @@

protected

  FMI.Internal.ExternalFMU instance = FMI.Internal.ExternalFMU();
@@ block equations @@
@@ endblock @@

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{@=x0=@,@=y0=@}, {@=x1=@,@=y1=@}}),
      graphics={
        Text(extent={{@=x0=@,@=y1+10=@}, {@=x1=@,@=y1+50=@}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{@=x0=@,@=y0=@},{@=x1=@,@=y1=@}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{@=x0=@,@=y0=@}, {@=x1=@,@=y1=@}})),
    experiment(StopTime=@=stopTime=@),
    uses(FMI(version="@=libraryVersion=@")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://@=rootPackage=@/Resources/FMUs/@=hash=@/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end @=modelName=@;
