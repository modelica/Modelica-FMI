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

  FMI3Float64Output h(unit="m", quantity="Position") "Position of the ball" annotation(Placement(transformation(extent={ { 100, -60 }, { 120, -40 } }), iconTransformation(extent={ { 100, -60 }, { 120, -40 } })));

  FMI3Float64Output v(unit="m/s", quantity="Velocity") "Velocity of the ball" annotation(Placement(transformation(extent={ { 100, 40 }, { 120, 60 } }), iconTransformation(extent={ { 100, 40 }, { 120, 60 } })));

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

  FMI3SetFloat64(instance, valueReferences={5}, values={g});
  FMI3SetFloat64(instance, valueReferences={6}, values={e});

  startTime := time;

  FMI3EnterInitializationMode(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);

  FMI3ExitInitializationMode(instance);

algorithm

  when sample(startTime, communicationStepSize) then

    

    FMI3DoStep(instance,
        currentCommunicationPoint=time,
        communicationStepSize=communicationStepSize);

    h := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
    v := scalar(FMI3GetFloat64(instance, valueReference=3, nValues=1));

  end when;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=3.0),
    uses(FMI(version="0.0.8")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/01cb20e/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end BouncingBall;