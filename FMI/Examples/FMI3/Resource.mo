within FMI.Examples.FMI3;

block Resource
  "This model loads data from a resource file"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;


  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  FMI3Int32Output y "Value of the first character in resources/y.txt" annotation(Placement(transformation(extent={ { 600, -10.0 }, { 620, 10.0 } }), iconTransformation(extent={ { 600, -10.0 }, { 620, 10.0 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    FMI3Int32 y;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/6af1132"),
    fmiVersion=3,
    modelIdentifier="Resource",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{7b9c2114-2ce5-4076-a138-2cbc69e069e5}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
    FMI3EnterInitializationMode(instance,
        toleranceDefined=tolerance > 0.0,
        tolerance=tolerance,
        startTime=startTime,
        stopTimeDefined=stopTime < Modelica.Constants.inf,
        stopTime=stopTime);
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then


    if not initialized and not initial() then
      FMI3ExitInitializationMode(instance);
      initialized := true;
    end if;

    if time >= startTime + communicationStepSize then
      FMI3DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize);
    end if;

    if not initial() then
      outputVariables.y := scalar(FMI3GetInt32(instance, valueReference=1, nValues=1));
    end if;

  end when;

equation

algorithm

  if initial() then
    y := scalar(FMI3GetInt32(instance, 1, 1));
  else
    y := outputVariables.y;
  end if;

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-600,-100}, {600,100}}),
      graphics={
        Text(extent={{-600,110}, {600,150}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{-600,-100},{600,100}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-600,-100}, {600,100}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/6af1132/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Resource;