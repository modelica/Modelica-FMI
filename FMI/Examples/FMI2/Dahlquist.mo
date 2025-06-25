within FMI.Examples.FMI2;

block Dahlquist
  "This model implements the Dahlquist test equation"
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.1 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI2Real k = 1;

  FMI2RealOutput x "the only state" annotation(Placement(transformation(extent={ { 100, -10 }, { 120, 10 } }), iconTransformation(extent={ { 100, -10 }, { 120, 10 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    Real x;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/48c7550"),
    fmiVersion=2,
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
    FMI2SetReal(instance, valueReferences={3}, nValues=1, values={k});
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
      outputVariables.x := FMI2GetReal(instance, valueReference=1);
    end if;

  end when;

equation

  if initial() then
    x = pure(FMI2GetInitialReal(instance, startTime, valueReference=1));
  else
    x = outputVariables.x;
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
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/48c7550/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Dahlquist;