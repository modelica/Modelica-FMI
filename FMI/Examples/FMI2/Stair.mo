within FMI.Examples.FMI2;

block Stair
  "This model generates a stair signal using time events"
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.2 annotation(Dialog(tab="FMI", group="Parameters"));

  FMI2IntegerOutput counter "counts the seconds" annotation(Placement(transformation(extent={ { 100, -10 }, { 120, 10 } }), iconTransformation(extent={ { 100, -10 }, { 120, 10 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    Integer counter;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/29c23fe"),
    fmiVersion=2,
    modelIdentifier="Stair",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{BD403596-3166-4232-ABC2-132BDF73E644}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
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
      outputVariables.counter := FMI2GetInteger(instance, valueReference=1);
    end if;

  end when;

equation

algorithm

  if initial() then
    counter := FMI2GetInteger(instance, valueReference=1);
  else
    counter := outputVariables.counter;
  end if;

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
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/29c23fe/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Stair;