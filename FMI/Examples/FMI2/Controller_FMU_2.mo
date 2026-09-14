within FMI.Examples.FMI2;
block Controller_FMU_2
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));
  parameter FMI2Real PI_k = 0.1 "Gain";
  parameter FMI2Real PI_T = 0.005 "Time Constant (T>0 required)";
  parameter FMI2Real PI_x_start = 0 "Initial or guess value of state";
  parameter FMI2Real PI_y_start = 0 "Initial value of output";
  parameter FMI2Real w_desired_start = 0.0 annotation(Dialog(tab="Initial", group="Start Values"));
  parameter FMI2Real w_start = 0.0 annotation(Dialog(tab="Initial", group="Start Values"));
  FMI2RealInput w_desired(start=w_desired_start) annotation(Placement(transformation(extent={ { -120, 70},  { -100, 90}}),   iconTransformation(extent={ { -120, 70},  { -100, 90}})));
  FMI2RealInput w(start=w_start) annotation(Placement(transformation(extent={ { -120, -90},  { -100, -70}}),   iconTransformation(extent={ { -120, -90},  { -100, -70}})));
  FMI2RealOutput V annotation(Placement(transformation(extent={ { 100, -10},  { 120, 10}}),   iconTransformation(extent={ { 100, -10},  { 120, 10}})));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/13087dc"),
    fmiVersion=2,
    modelIdentifier="Controller_FMU_2",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{ccbc3e0a-d9ec-4288-ba73-8f9d38827f57}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  startTime := time;

  FMI2SetReal(instance, valueReferences={ 16777216},  nValues=1, values={ PI_k});
  FMI2SetReal(instance, valueReferences={ 16777217},  nValues=1, values={ PI_T});
  FMI2SetReal(instance, valueReferences={ 16777218},  nValues=1, values={ PI_x_start});
  FMI2SetReal(instance, valueReferences={ 16777219},  nValues=1, values={ PI_y_start});

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
    FMI2SetReal(instance, valueReferences={ 620756992},  nValues=1, values={ pre(w_desired)});
    FMI2SetReal(instance, valueReferences={ 620756993},  nValues=1, values={ pre(w)});

    if time >= startTime + communicationStepSize then
      FMI2DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize,
        noSetFMUStatePriorToCurrentPoint=true);
    end if;

    V := FMI2GetReal(instance, valueReference=603979776);

  end when;

  annotation (
    Icon(
      coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}}),
      graphics={Bitmap(extent={ {-90, -90}, {90, 90}},  fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html><p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/13087dc/documentation/index.html\">original documentation</a>.</p></html>"));
end Controller_FMU_2;
