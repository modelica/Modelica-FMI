within FMI.Examples.FMI3;

block Dahlquist
  "This model implements the Dahlquist test equation"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;


  parameter Modelica.Units.SI.Time communicationStepSize = 0.1 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 k = 1;

  FMI3Float64Output x "the only state" annotation(Placement(transformation(extent={ { 600, -10.0 }, { 620, 10.0 } }), iconTransformation(extent={ { 600, -10.0 }, { 620, 10.0 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    FMI3Float64 x;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/f75d448"),
    fmiVersion=3,
    modelIdentifier="Dahlquist",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{221063D2-EF4A-45FE-B954-B5BFEEA9A59B}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
    FMI3SetFloat64(instance, valueReferences={3}, values={k});
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
      outputVariables.x := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
    end if;

  end when;

equation

  if initial() then
    x = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=1, nValues=1)));
  else
    x = outputVariables.x;
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
    experiment(StopTime=10.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/f75d448/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Dahlquist;