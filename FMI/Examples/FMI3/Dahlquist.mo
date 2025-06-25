within FMI.Examples.FMI3;

block Dahlquist
  "This model implements the Dahlquist test equation"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.1 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 k = 1;

  FMI3Float64Output x "the only state" annotation(Placement(transformation(extent={ { 100, -10 }, { 120, 10 } }), iconTransformation(extent={ { 100, -10 }, { 120, 10 } })));

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

  FMI3SetFloat64(instance, valueReferences={3}, values={k});

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

    x := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));

  end when;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=10.0),
    uses(FMI(version="0.0.8")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/f75d448/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Dahlquist;