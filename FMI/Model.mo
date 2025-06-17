within FMI;

block Model
  "A test model with 5 parameters, 5 inputs, 5 outputs, and 5 states."
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;


  parameter Modelica.Units.SI.Time communicationStepSize = 0.001 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 p0 = 0 "Parameter 0";

  parameter FMI3Float64 p1 = 0 "Parameter 1";

  parameter FMI3Float64 p2 = 0 "Parameter 2";

  parameter FMI3Float64 p3 = 0 "Parameter 3";

  parameter FMI3Float64 p4 = 0 "Parameter 4";

  FMI3Float64Input u0 annotation (Placement(transformation(extent={ { -640, 146.66666666666669 }, { -600, 186.66666666666669 } }), iconTransformation(extent={ { -640, 146.66666666666669 }, { -600, 186.66666666666669 } })));

  FMI3Float64Input u1 annotation (Placement(transformation(extent={ { -640, 63.33333333333334 }, { -600, 103.33333333333334 } }), iconTransformation(extent={ { -640, 63.33333333333334 }, { -600, 103.33333333333334 } })));

  FMI3Float64Input u2 annotation (Placement(transformation(extent={ { -640, -20.0 }, { -600, 20.0 } }), iconTransformation(extent={ { -640, -20.0 }, { -600, 20.0 } })));

  FMI3Float64Input u3 annotation (Placement(transformation(extent={ { -640, -103.33333333333331 }, { -600, -63.333333333333314 } }), iconTransformation(extent={ { -640, -103.33333333333331 }, { -600, -63.333333333333314 } })));

  FMI3Float64Input u4 annotation (Placement(transformation(extent={ { -640, -186.66666666666663 }, { -600, -146.66666666666663 } }), iconTransformation(extent={ { -640, -186.66666666666663 }, { -600, -146.66666666666663 } })));

  FMI3Float64Output y0 annotation (Placement(transformation(extent={ { 600, 156.66666666666669 }, { 620, 176.66666666666669 } }), iconTransformation(extent={ { 600, 156.66666666666669 }, { 620, 176.66666666666669 } })));

  FMI3Float64Output y1 annotation (Placement(transformation(extent={ { 600, 73.33333333333334 }, { 620, 93.33333333333334 } }), iconTransformation(extent={ { 600, 73.33333333333334 }, { 620, 93.33333333333334 } })));

  FMI3Float64Output y2 annotation (Placement(transformation(extent={ { 600, -10.0 }, { 620, 10.0 } }), iconTransformation(extent={ { 600, -10.0 }, { 620, 10.0 } })));

  FMI3Float64Output y3 annotation (Placement(transformation(extent={ { 600, -93.33333333333331 }, { 620, -73.33333333333331 } }), iconTransformation(extent={ { 600, -93.33333333333331 }, { 620, -73.33333333333331 } })));

  FMI3Float64Output y4 annotation (Placement(transformation(extent={ { 600, -176.66666666666663 }, { 620, -156.66666666666663 } }), iconTransformation(extent={ { 600, -176.66666666666663 }, { 620, -156.66666666666663 } })));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/72bbf4d"),
    fmiVersion=3,
    modelIdentifier="model",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="0fbad382-02da-4cdf-ad81-c92aa58cf00b",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  FMI3SetFloat64(instance, valueReferences={1}, values={p0});
  FMI3SetFloat64(instance, valueReferences={2}, values={p1});
  FMI3SetFloat64(instance, valueReferences={3}, values={p2});
  FMI3SetFloat64(instance, valueReferences={4}, values={p3});
  FMI3SetFloat64(instance, valueReferences={5}, values={p4});

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

    FMI3SetFloat64(instance, valueReferences={6}, values={u0});
    FMI3SetFloat64(instance, valueReferences={7}, values={u1});
    FMI3SetFloat64(instance, valueReferences={8}, values={u2});
    FMI3SetFloat64(instance, valueReferences={9}, values={u3});
    FMI3SetFloat64(instance, valueReferences={10}, values={u4});

    FMI3DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize);

    y0 := scalar(FMI3GetFloat64(instance, valueReference=11, nValues=1));
    y1 := scalar(FMI3GetFloat64(instance, valueReference=12, nValues=1));
    y2 := scalar(FMI3GetFloat64(instance, valueReference=13, nValues=1));
    y3 := scalar(FMI3GetFloat64(instance, valueReference=14, nValues=1));
    y4 := scalar(FMI3GetFloat64(instance, valueReference=15, nValues=1));

  end when;

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-600,-250}, {600,250}}),
      graphics={
        Text(extent={{-600,260}, {600,300}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{-600,-250},{600,250}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-600,-250}, {600,250}})),
    experiment(StopTime=1.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/72bbf4d/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Model;