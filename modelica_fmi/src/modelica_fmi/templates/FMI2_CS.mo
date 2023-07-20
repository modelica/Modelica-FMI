@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;
@@ endblock @@
@@ block parameters @@

  parameter Modelica.Units.SI.Time communicationStepSize = @=communicationStepSize=@ annotation(Dialog(tab="FMI", group="Parameters"));
@@ endblock @@
@@ block equations @@

protected

  parameter Real startTime(fixed=false);
  parameter Boolean startValuesSet(start=false, fixed=false);
  Boolean initialized(start=false, fixed=true);

  record OutputVariables
@@ for variable in outputs @@
    @=fmi_type(variable)=@ @=name(variable)=@;
@@ endfor @@
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://@=rootPackage=@/Resources/FMUs/@=hash=@"),
    fmiVersion=@=fmiMajorVersion=@,
    modelIdentifier="@=modelIdentifier=@",
    instanceName=getInstanceName(),
    interfaceType=@=interfaceType=@,
    instantiationToken="@=instantiationToken=@",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=@@ if copyPlatformBinary @@true@@ else @@false@@ endif @@);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
@@ for variable in parameters @@
    FMI2Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, nValues=1, values={@=name(variable)=@});
@@ endfor @@
    FMI2SetupExperiment(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);
    FMI2EnterInitializationMode(instance);
@@ for variable in inputs @@
    FMI2Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, 1, {@=name(variable, '_start')=@});
@@ endfor @@
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

@@ for variable in inputs @@
    FMI2Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, 1, {pre(@=name(variable)=@)});
@@ endfor @@

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
@@ for variable in outputs @@
      outputVariables.@=name(variable)=@ := FMI2Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@);
@@ endfor @@
    end if;

  end when;

equation
@@ for variable in continuousOutputs @@

  if initial() then
    @=name(variable)=@ = FMI2GetInitialReal(instance, startTime, @=dependencies3(variable, 'Real')=@@=dependencies3(variable, 'Integer')=@@=dependencies3(variable, 'Boolean')=@valueReference=@=variable.valueReference=@);
  else
    @=name(variable)=@ = outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ for variable in discreteOutputs @@

algorithm

  if initial() then
@@ for dependency in dependencies2(variable) @@
    FMI2Set@=fmi_type(dependency)=@(instance, {@=dependency.valueReference=@}, 1, {@=name(dependency)=@});
@@ endfor @@
    @=name(variable)=@ := FMI2Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@);
  else
    @=name(variable)=@ := outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ endblock @@