within FMI.Examples.FMI2;
block Drive_FMU_2
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));
  parameter FMI2Real loadInertia1_J = 1 "Moment of inertia";
  parameter FMI2Real idealGear_ratio = 10 "Transmission ratio (flange_a.phi/flange_b.phi)";
  parameter FMI2Real dcpm_TaOperational = 293.15 "Operational armature temperature";
  parameter FMI2Real dcpm_fixed_phi0 = 0 "Fixed offset angle of housing";
  parameter FMI2Real dcpmData_Jr = 0.001 "Rotor's moment of inertia";
  parameter FMI2Real dcpmData_VaNominal = 100 "Nominal armature voltage";
  parameter FMI2Real dcpmData_IaNominal = 100 "Nominal armature current (>0..Motor, <0..Generator)";
  parameter FMI2Real dcpmData_wNominal = 149.22565104552 "Nominal speed";
  parameter FMI2Real dcpmData_TaNominal = 293.15 "Nominal armature temperature";
  parameter FMI2Real dcpmData_Ra = 0.05 "Armature resistance at TaRef";
  parameter FMI2Real dcpmData_TaRef = 293.15 "Reference temperature of armature resistance";
  parameter FMI2Real dcpmData_alpha20a = 0 "Temperature coefficient of armature resistance";
  parameter FMI2Real dcpmData_La = 0.0015 "Armature inductance";
  parameter FMI2Real dcpmData_frictionParameters_power_w = 2 "Exponent of friction torque w.r.t. angular velocity";
  parameter FMI2Real dcpmData_strayLoadParameters_power_w = 1 "Exponent of stray load loss torque w.r.t. angular velocity";
  parameter FMI2Real V_start = 0.0 annotation(Dialog(tab="Initial", group="Start Values"));
  parameter FMI2Real LoadTorque_Nm_start = 0.0 annotation(Dialog(tab="Initial", group="Start Values"));
  FMI2RealInput V(start=V_start) annotation(Placement(transformation(extent={ { -120, 70},  { -100, 90}}),   iconTransformation(extent={ { -120, 70},  { -100, 90}})));
  FMI2RealInput LoadTorque_Nm(start=LoadTorque_Nm_start) annotation(Placement(transformation(extent={ { -120, -90},  { -100, -70}}),   iconTransformation(extent={ { -120, -90},  { -100, -70}})));
  FMI2RealOutput w annotation(Placement(transformation(extent={ { 100, -10},  { 120, 10}}),   iconTransformation(extent={ { 100, -10},  { 120, 10}})));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/f12c410"),
    fmiVersion=2,
    modelIdentifier="Drive_FMU_2",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{00c03ef7-4b7c-4917-8c9e-7a2416506381}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  startTime := time;

  FMI2SetReal(instance, valueReferences={ 16777216},  nValues=1, values={ loadInertia1_J});
  FMI2SetReal(instance, valueReferences={ 16777217},  nValues=1, values={ idealGear_ratio});
  FMI2SetReal(instance, valueReferences={ 16777218},  nValues=1, values={ dcpm_TaOperational});
  FMI2SetReal(instance, valueReferences={ 16777219},  nValues=1, values={ dcpm_fixed_phi0});
  FMI2SetReal(instance, valueReferences={ 16777220},  nValues=1, values={ dcpmData_Jr});
  FMI2SetReal(instance, valueReferences={ 16777221},  nValues=1, values={ dcpmData_VaNominal});
  FMI2SetReal(instance, valueReferences={ 16777222},  nValues=1, values={ dcpmData_IaNominal});
  FMI2SetReal(instance, valueReferences={ 16777223},  nValues=1, values={ dcpmData_wNominal});
  FMI2SetReal(instance, valueReferences={ 16777224},  nValues=1, values={ dcpmData_TaNominal});
  FMI2SetReal(instance, valueReferences={ 16777225},  nValues=1, values={ dcpmData_Ra});
  FMI2SetReal(instance, valueReferences={ 16777226},  nValues=1, values={ dcpmData_TaRef});
  FMI2SetReal(instance, valueReferences={ 16777227},  nValues=1, values={ dcpmData_alpha20a});
  FMI2SetReal(instance, valueReferences={ 16777228},  nValues=1, values={ dcpmData_La});
  FMI2SetReal(instance, valueReferences={ 16777229},  nValues=1, values={ dcpmData_frictionParameters_power_w});
  FMI2SetReal(instance, valueReferences={ 16777230},  nValues=1, values={ dcpmData_strayLoadParameters_power_w});

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
    FMI2SetReal(instance, valueReferences={ 620756992},  nValues=1, values={ pre(V)});
    FMI2SetReal(instance, valueReferences={ 620756993},  nValues=1, values={ pre(LoadTorque_Nm)});

    if time >= startTime + communicationStepSize then
      FMI2DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize,
        noSetFMUStatePriorToCurrentPoint=true);
    end if;

    w := FMI2GetReal(instance, valueReference=603979776);

  end when;

  annotation (
    Icon(
      coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}}),
      graphics={Bitmap(extent={ {-90, -90}, {90, 90}},  fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html><p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/f12c410/documentation/index.html\">original documentation</a>.</p></html>"));
end Drive_FMU_2;
