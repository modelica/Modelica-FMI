within FMI.Examples.FMI2;

block BouncingBall
  "This model calculates the trajectory, over time, of a ball dropped from a height of 1 m"
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;


  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI2Real g(unit="m/s2", quantity="Acceleration") = -9.81 "Gravity acting on the ball";

  parameter FMI2Real e = 0.7 "Coefficient of restitution";

  FMI2RealOutput h(unit="m", quantity="Position") "Position of the ball" annotation(Placement(transformation(extent={ { 600, 23.33333333333333 }, { 620, 43.33333333333333 } }), iconTransformation(extent={ { 600, 23.33333333333333 }, { 620, 43.33333333333333 } })));

  FMI2RealOutput v(unit="m/s", quantity="Velocity") "Velocity of the ball" annotation(Placement(transformation(extent={ { 600, -43.33333333333334 }, { 620, -23.333333333333343 } }), iconTransformation(extent={ { 600, -43.33333333333334 }, { 620, -23.333333333333343 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    Real h;
    Real v;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/c768b51"),
    fmiVersion=2,
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
    FMI2SetReal(instance, valueReferences={5}, nValues=1, values={g});
    FMI2SetReal(instance, valueReferences={6}, nValues=1, values={e});
    FMI2SetupExperiment(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);
    FMI2EnterInitializationMode(instance);
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then


    if not initialized and not initial() then
      FMI2ExitInitializationMode(instance);
      initialized := true;
    end if;

    if time >= startTime + communicationStepSize then
      FMI2DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize,
        noSetFMUStatePriorToCurrentPoint=true);
    end if;

    if not initial() then
      outputVariables.h := FMI2GetReal(instance, valueReference=1);
      outputVariables.v := FMI2GetReal(instance, valueReference=3);
    end if;

  end when;

equation

  if initial() then
    h = pure(FMI2GetInitialReal(instance, startTime, valueReference=1));
  else
    h = outputVariables.h;
  end if;

  if initial() then
    v = pure(FMI2GetInitialReal(instance, startTime, valueReference=3));
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
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/c768b51/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end BouncingBall;