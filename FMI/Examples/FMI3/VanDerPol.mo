within FMI.Examples.FMI3;

block VanDerPol
  "This model implements the van der Pol oscillator"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

  parameter Modelica.Units.SI.Time communicationStepSize = 0.01 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 mu = 1;

  FMI3Float64Output x0 "the first state" annotation(Placement(transformation(extent={ { 100, -60 }, { 120, -40 } }), iconTransformation(extent={ { 100, -60 }, { 120, -40 } })));

  FMI3Float64Output x1 "the second state" annotation(Placement(transformation(extent={ { 100, 40 }, { 120, 60 } }), iconTransformation(extent={ { 100, 40 }, { 120, 60 } })));

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

  FMI3SetFloat64(instance, valueReferences={5}, values={mu});

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

    

    FMI3DoStep(instance,
        currentCommunicationPoint=time,
        communicationStepSize=communicationStepSize);

    x0 := scalar(FMI3GetFloat64(instance, valueReference=1, nValues=1));
    x1 := scalar(FMI3GetFloat64(instance, valueReference=3, nValues=1));

  end when;

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
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/fe51475/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end VanDerPol;