within FMI.Examples.FMI3;

block BouncingBall
  "This model calculates the trajectory, over time, of a ball dropped from a height of 1 m"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 g(unit="m/s2", quantity="Acceleration") = -9.81 "Gravity acting on the ball";

  parameter FMI3Float64 e = 0.7 "Coefficient of restitution";

  FMI3Float64Output h(unit="m", quantity="Position") "Position of the ball" annotation(Placement(transformation(extent={ { 100, 70.0},  { 120, 90.0}}),   iconTransformation(extent={ { 100, 70.0},  { 120, 90.0}})));

  FMI3Float64Output v(unit="m/s", quantity="Velocity") "Velocity of the ball" annotation(Placement(transformation(extent={ { 100, -10.0},  { 120, 10.0}}),   iconTransformation(extent={ { 100, -10.0},  { 120, 10.0}})));

  FMI3Float64Output h_ft(unit="m", quantity="Position") "Position in feet" annotation(Placement(transformation(extent={ { 100, -90.0},  { 120, -70.0}}),   iconTransformation(extent={ { 100, -90.0},  { 120, -70.0}})));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    FMI3Float64 h;
    FMI3Float64 v;
    FMI3Float64 h_ft;
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
      outputVariables.h_ft := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
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

  if initial() then
    h_ft = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=1, nValues=1)));
  else
    h_ft = outputVariables.h_ft;
  end if;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=3),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/01cb20e/documentation/index.html\">original documentation</a>.</p>
</html>"));
end BouncingBall;
