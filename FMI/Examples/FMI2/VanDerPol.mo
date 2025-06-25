within FMI.Examples.FMI2;

block VanDerPol
  "This model implements the van der Pol oscillator"
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI2Real mu = 1;

  FMI2RealOutput x0 "the first state" annotation(Placement(transformation(extent={ { 100, -60 }, { 120, -40 } }), iconTransformation(extent={ { 100, -60 }, { 120, -40 } })));

  FMI2RealOutput x1 "the second state" annotation(Placement(transformation(extent={ { 100, 40 }, { 120, 60 } }), iconTransformation(extent={ { 100, 40 }, { 120, 60 } })));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    Real x0;
    Real x1;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/a2005ef"),
    fmiVersion=2,
    modelIdentifier="VanDerPol",
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
    FMI2SetReal(instance, valueReferences={5}, nValues=1, values={mu});
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
      outputVariables.x0 := FMI2GetReal(instance, valueReference=1);
      outputVariables.x1 := FMI2GetReal(instance, valueReference=3);
    end if;

  end when;

equation

  if initial() then
    x0 = pure(FMI2GetInitialReal(instance, startTime, valueReference=1));
  else
    x0 = outputVariables.x0;
  end if;

  if initial() then
    x1 = pure(FMI2GetInitialReal(instance, startTime, valueReference=3));
  else
    x1 = outputVariables.x1;
  end if;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=20.0),
    uses(FMI(version="0.0.8")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/a2005ef/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end VanDerPol;