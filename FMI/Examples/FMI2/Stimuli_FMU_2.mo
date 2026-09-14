within FMI.Examples.FMI2;
block Stimuli_FMU_2
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));
  parameter FMI2Real stepTau_height = 3 "Height of step";
  parameter FMI2Real stepTau_offset = 0 "Offset of output signal y";
  parameter FMI2Real stepTau_startTime = 0.5 "Output y = offset for time < startTime";
  parameter FMI2Real stepW_height = 10 "Height of step";
  parameter FMI2Real stepW_offset = 0 "Offset of output signal y";
  parameter FMI2Real stepW_startTime = 0.1 "Output y = offset for time < startTime";
  FMI2RealOutput w_desired annotation(Placement(transformation(extent={ { 100, -60},  { 120, -40}}),   iconTransformation(extent={ { 100, -60},  { 120, -40}})));
  FMI2RealOutput LoadTorque_Nm annotation(Placement(transformation(extent={ { 100, 40},  { 120, 60}}),   iconTransformation(extent={ { 100, 40},  { 120, 60}})));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/8f1de8f"),
    fmiVersion=2,
    modelIdentifier="Stimuli_FMU_2",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{0fcdd736-ea18-4399-9284-991316e6e42f}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  startTime := time;

  FMI2SetReal(instance, valueReferences={ 16777216},  nValues=1, values={ stepTau_height});
  FMI2SetReal(instance, valueReferences={ 16777217},  nValues=1, values={ stepTau_offset});
  FMI2SetReal(instance, valueReferences={ 16777218},  nValues=1, values={ stepTau_startTime});
  FMI2SetReal(instance, valueReferences={ 16777219},  nValues=1, values={ stepW_height});
  FMI2SetReal(instance, valueReferences={ 16777220},  nValues=1, values={ stepW_offset});
  FMI2SetReal(instance, valueReferences={ 16777221},  nValues=1, values={ stepW_startTime});

  FMI2SetupExperiment(instance,
    toleranceDefined=tolerance > 0.0,
    tolerance=tolerance,
    startTime=startTime,
    stopTimeDefined=stopTime < Modelica.Constants.inf,
    stopTime=stopTime);

  FMI2EnterInitializationMode(instance);

  FMI2ExitInitializationMode(instance);

algorithm

  when sample(startTime, communicationStepSize) then

    if time >= startTime + communicationStepSize then
      FMI2DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize,
        noSetFMUStatePriorToCurrentPoint=true);
    end if;

    w_desired := FMI2GetReal(instance, valueReference=603979776);
    LoadTorque_Nm := FMI2GetReal(instance, valueReference=603979777);

  end when;

  annotation (
    Icon(
      coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}}),
      graphics={Bitmap(extent={ {-90, -90}, {90, 90}},  fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html><p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/8f1de8f/documentation/index.html\">original documentation</a>.</p></html>"));
end Stimuli_FMU_2;
