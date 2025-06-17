within FMI.Examples.FMI3;

block BouncingBall
  "This model calculates the trajectory, over time, of a ball dropped from a height of 1 m"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;


  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 g(unit="m/s2", quantity="Acceleration") = -9.81 "Gravity acting on the ball";

  parameter FMI3Float64 e = 0.7 "Coefficient of restitution";

  FMI3Float64Output h(unit="m", quantity="Position") annotation (Placement(transformation(extent={ { 600, 23.33333333333333 }, { 620, 43.33333333333333 } }), iconTransformation(extent={ { 600, 23.33333333333333 }, { 620, 43.33333333333333 } })));

  FMI3Float64Output v(unit="m/s", quantity="Velocity") annotation (Placement(transformation(extent={ { 600, -43.33333333333334 }, { 620, -23.333333333333343 } }), iconTransformation(extent={ { 600, -43.33333333333334 }, { 620, -23.333333333333343 } })));

protected

  record OutputVariables
    FMI3Float64 h;
    FMI3Float64 v;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/01cb20e"),
    fmiVersion=3,
    modelIdentifier="BouncingBall",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{1AE5E10D-9521-4DE3-80B9-D0EAAA7D5AF1}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
    FMI3SetFloat64(instance, valueReferences={5}, values={g});
    FMI3SetFloat64(instance, valueReferences={6}, values={e});
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
      outputVariables.h := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
      outputVariables.v := scalar(FMI3GetFloat64(instance, valueReference=3, nValues=1));
    end if;

  end when;

equation

  if initial() then
    h = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=1, nValues=1)));
  else
    h = outputVariables.h;
  end if;

  if initial() then
    v = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=3, nValues=1)));
  else
    v = outputVariables.v;
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
    experiment(StopTime=3.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/01cb20e/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end BouncingBall;