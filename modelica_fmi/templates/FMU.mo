within @=package=@;

model @=modelName=@
@@ if description @@
  "@=description=@"
@@ endif @@

@@ block imports @@
@@ endblock @@

  parameter Modelica.Units.SI.Time startTime = 0.0 annotation(Dialog(tab="FMI", group="Parameters"));

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

  parameter @=variable.type=@ '@=variable.name=@'@=attributes(variable)=@ = @=start_value(variable)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@;
@@ endfor @@
@@ for variable in inputs @@

  parameter @=variable.type=@ '@=variable.name=@_start'@=attributes(variable)=@ = @=start_value(variable)=@;
@@ endfor @@
@@ for variable in inputs @@

  @=variable.type=@Input '@=variable.name=@'@=attributes(variable, start=True)=@ @=annotations[variable.name]=@;
@@ endfor @@
@@ for variable in outputs @@

  @=variable.type=@Output '@=variable.name=@'@=attributes(variable)=@ @=annotations[variable.name]=@;
@@ endfor @@

protected

  FMI.Internal.ExternalFMU instance = FMI.Internal.ExternalFMU(
    Modelica.Utilities.Files.loadResource("modelica://@=rootPackage=@/Resources/FMUs/@=hash=@"),
    @=fmiMajorVersion=@,
    "@=modelIdentifier=@",
    getInstanceName(),
    @=interfaceType=@,
    "@=instantiationToken=@",
    visible,
    loggingOn,
    logFMICalls,
    logToFile,
    logFile);
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
    experiment(StopTime=@=stopTime=@)
  );
end @=modelName=@;
