within FMI.Examples.FMI3;

block Resource
  "This model loads data from a resource file"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  FMI3Int32Output y "Value of the first character in resources/y.txt" annotation(Placement(transformation(extent={ { 100, -10 }, { 120, 10 } }), iconTransformation(extent={ { 100, -10 }, { 120, 10 } })));

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

    y := scalar(FMI3GetInt32(instance, valueReference=1, nValues=1));

  end when;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.8")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/6af1132/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Resource;