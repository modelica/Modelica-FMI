within FMI.Examples.FMI3;

block VanDerPol
  "This model implements the van der Pol oscillator"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 mu = 1;

  FMI3Float64Output x0 "the first state" annotation(Placement(transformation(extent={ { 100, -60},  { 120, -40}}),   iconTransformation(extent={ { 100, -60},  { 120, -40}})));

  FMI3Float64Output x1 "the second state" annotation(Placement(transformation(extent={ { 100, 40},  { 120, 60}}),   iconTransformation(extent={ { 100, 40},  { 120, 60}})));

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
    FMI3Float64 x0;
    FMI3Float64 x1;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/fe51475"),
    fmiVersion=3,
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
    FMI3SetFloat64(instance, valueReferences={5}, values={mu});
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
      outputVariables.x0 := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
      outputVariables.x1 := scalar(FMI3GetFloat64(instance, valueReference=3, nValues=1));
    end if;

  end when;

equation

  if initial() then
    x0 = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=1, nValues=1)));
  else
    x0 = outputVariables.x0;
  end if;

  if initial() then
    x1 = scalar(pure(FMI3GetInitialFloat64(instance, startTime, valueReference=3, nValues=1)));
  else
    x1 = outputVariables.x1;
  end if;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=20),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/fe51475/documentation/index.html\">original documentation</a>.</p>
</html>"));
end VanDerPol;
