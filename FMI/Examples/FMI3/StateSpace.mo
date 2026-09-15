within FMI.Examples.FMI3;

block StateSpace
  "This model implements a linear time-invariant (LTI) system"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));
  parameter FMI3Float64 A[3,3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}} "Matrix coefficient A";
  parameter FMI3Float64 B[3,3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}} "Matrix coefficient B";
  parameter FMI3Float64 C[3,3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}} "Matrix coefficient C";
  parameter FMI3Float64 D[3,3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}} "Matrix coefficient D";
  parameter FMI3Float64 x0[3] = {0, 0, 0} "Initial state vector";
  parameter FMI3Float64 u_start[3] = {1, 2, 3} annotation(Dialog(tab="Initial", group="Start Values"));
  FMI3Float64Input u[3](start=u_start) "Input vector" annotation(Placement(transformation(extent={ { -120, -60 }, { -100, -40 } }), iconTransformation(extent={ { -120, -60 }, { -100, -40 } })));
  FMI3Float64Output y[3] "Output vector" annotation(Placement(transformation(extent={ { 100, 40 }, { 120, 60 } }), iconTransformation(extent={ { 100, 40 }, { 120, 60 } })));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/b4d2e6a"),
    fmiVersion=3,
    modelIdentifier="StateSpace",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{D773325B-AB94-4630-BF85-643EB24FCB78}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  startTime := time;

  FMI3SetFloat64(instance, valueReferences={ 4 }, values={ A });
  FMI3SetFloat64(instance, valueReferences={ 5 }, values={ B });
  FMI3SetFloat64(instance, valueReferences={ 6 }, values={ C });
  FMI3SetFloat64(instance, valueReferences={ 7 }, values={ D });
  FMI3SetFloat64(instance, valueReferences={ 8 }, values={ x0 });

  FMI3EnterInitializationMode(instance,
    toleranceDefined=tolerance > 0.0,
    tolerance=tolerance,
    startTime=startTime,
    stopTimeDefined=stopTime < Modelica.Constants.inf,
    stopTime=stopTime);

  FMI3ExitInitializationMode(instance);

algorithm

  when sample(startTime, communicationStepSize) then
    FMI3SetFloat64(instance, valueReferences={ 9 }, values={ pre(u) });

    if time >= startTime + communicationStepSize then
      FMI3DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize);
    end if;

    y := scalar(FMI3GetFloat64(instance, valueReference=10, nValues=1));

  end when;

  annotation (
    Icon(
      coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100} }),
      graphics={Bitmap(extent={ {-90, -90}, {90, 90} }, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={ {-100, -100}, {100, 100} })),
    experiment(StopTime=10),
    uses(FMI(version="0.0.9")),
    Documentation(info="<html><p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/b4d2e6a/documentation/index.html\">original documentation</a>.</p></html>")
  );
end StateSpace;