within FMI.Examples.FMI3;

block StateSpace
  "This model implements a linear time-invariant (LTI) system"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 A[3,3] = [1, 0, 0; 0, 1, 0; 0, 0, 1] "Matrix coefficient A";

  parameter FMI3Float64 B[3,3] = [1, 0, 0; 0, 1, 0; 0, 0, 1] "Matrix coefficient B";

  parameter FMI3Float64 C[3,3] = [1, 0, 0; 0, 1, 0; 0, 0, 1] "Matrix coefficient C";

  parameter FMI3Float64 D[3,3] = [1, 0, 0; 0, 1, 0; 0, 0, 1] "Matrix coefficient D";

  parameter FMI3Float64 x0[3] = {0, 0, 0} "Initial state vector";

  FMI3Float64Input u[3] "Input vector" annotation(Placement(transformation(extent={ { -140, -20 }, { -100, 20 } }), iconTransformation(extent={ { -140, -20 }, { -100, 20 } })));

  FMI3Float64Output y[3] "Output vector" annotation(Placement(transformation(extent={ { 100, -10 }, { 120, 10 } }), iconTransformation(extent={ { 100, -10 }, { 120, 10 } })));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/d5a5454"),
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

  FMI3SetFloat64(instance, valueReferences={4}, values=cat(1, A[1,:], A[2,:], A[3,:]));
  FMI3SetFloat64(instance, valueReferences={5}, values=cat(1, B[1,:], B[2,:], B[3,:]));
  FMI3SetFloat64(instance, valueReferences={6}, values=cat(1, C[1,:], C[2,:], C[3,:]));
  FMI3SetFloat64(instance, valueReferences={7}, values=cat(1, D[1,:], D[2,:], D[3,:]));
  FMI3SetFloat64(instance, valueReferences={8}, values=x0);

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

    FMI3SetFloat64(instance, valueReferences={9}, values=u);

    FMI3DoStep(instance,
        currentCommunicationPoint=time,
        communicationStepSize=communicationStepSize);

    y := FMI3GetFloat64(instance, valueReference=10, nValues=3);

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
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/d5a5454/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end StateSpace;