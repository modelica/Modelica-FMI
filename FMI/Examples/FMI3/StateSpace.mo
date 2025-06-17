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

  parameter FMI3Float64 u_start[3] = {1, 2, 3} annotation(Dialog(tab="Initial", group="Start Values"));

  FMI3Float64Input u[3](start=u_start) annotation (Placement(transformation(extent={ { -640, -20.0 }, { -600, 20.0 } }), iconTransformation(extent={ { -640, -20.0 }, { -600, 20.0 } })));

  FMI3Float64Output y[3] annotation (Placement(transformation(extent={ { 600, -10.0 }, { 620, 10.0 } }), iconTransformation(extent={ { 600, -10.0 }, { 620, 10.0 } })));

protected

  record OutputVariables
    FMI3Float64 y[3];
  end OutputVariables;

  OutputVariables outputVariables;

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

  if not startValuesSet then
    startTime := time;
    FMI3SetFloat64Matrix(instance, valueReferences={4}, values=A, nValues=9);
    FMI3SetFloat64Matrix(instance, valueReferences={5}, values=B, nValues=9);
    FMI3SetFloat64Matrix(instance, valueReferences={6}, values=C, nValues=9);
    FMI3SetFloat64Matrix(instance, valueReferences={7}, values=D, nValues=9);
    FMI3SetFloat64(instance, valueReferences={8}, values=x0);
    FMI3EnterInitializationMode(instance,
        toleranceDefined=tolerance > 0.0,
        tolerance=tolerance,
        startTime=startTime,
        stopTimeDefined=stopTime < Modelica.Constants.inf,
        stopTime=stopTime);
    FMI3SetFloat64(instance, valueReferences={9}, values=u_start);
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

    FMI3SetFloat64(instance, valueReferences={9}, values=pre(u));

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
      outputVariables.y := FMI3GetFloat64(instance, valueReference=10, nValues=3);
    end if;

  end when;

equation

  if initial() then
    y = pure(FMI3GetInitialFloat64(instance, startTime, valueReference=10, nValues=size(y, 1)));
  else
    y = outputVariables.y;
  end if;

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-600,-100}, {600,100}}),
      graphics={
        Text(extent={{-600,110}, {600,150}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{-600,-100},{600,100}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-600,-100}, {600,100}})),
    experiment(StopTime=10.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/d5a5454/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end StateSpace;