@@ extends "FMU2.mo" @@
@@ block inputs @@
@@ for variable in inputs @@

  parameter @=fmi_type(variable, prefix=True, declared=True)=@ @=name(variable, '_start')=@@=subscripts(variable)=@@=modifiers(variable)=@ = @=start_value(variable)=@ annotation(Dialog(tab="Initial", group="Start Values"));
@@ endfor @@
@@ for variable in inputs @@

  @=fmi_type(variable, prefix=True, declared=True)=@Input @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable, start=True)=@ @=annotations[variable.name]=@;
@@ endfor @@
@@ endblock @@
@@ block equations @@

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
@@ for variable in outputs @@
    @=fmi_type(variable, declared=True)=@ @=name(variable)=@;
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
    FMI2Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, nValues=1, values={@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInteger(@@ endif @@@=name(variable)=@@@ if variable.type == 'Enumeration' @@)@@ endif @@});
@@ endfor @@
    FMI2SetupExperiment(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);
    FMI2EnterInitializationMode(instance);
@@ for variable in inputs @@
    FMI2Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, nValues=1, values={@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInteger(@@ endif @@@=name(variable, '_start')=@@@ if variable.type == 'Enumeration' @@)@@ endif @@});
@@ endfor @@
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

@@ for variable in inputs @@
    FMI2Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, nValues=1, values={@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInteger(@@ endif @@pre(@=name(variable)=@)@@ if variable.type == 'Enumeration' @@)@@ endif @@});
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
      outputVariables.@=name(variable)=@ := @@ if variable.type == 'Enumeration' @@Types.IntegerTo@= variable.declaredType.name =@(@@ endif @@FMI2Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@)@@ if variable.type == 'Enumeration' @@)@@ endif @@;
@@ endfor @@
    end if;

  end when;

equation
@@ for variable in continuousOutputs @@

  if initial() then
    @=name(variable)=@ = pure(FMI2GetInitialReal(instance, startTime, @=dependencies3(variable, ['Real', 'Integer', 'Boolean'])=@valueReference=@=variable.valueReference=@));
  else
    @=name(variable)=@ = outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ for variable in discreteOutputs @@

algorithm

  if initial() then
@@ for dependency in dependencies2(variable) @@
    FMI2Set@=fmi_type(dependency)=@(instance, valueReferences={@=dependency.valueReference=@}, nValues=1, values={@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInteger(@@ endif @@@=name(dependency)=@@@ if variable.type == 'Enumeration' @@)@@ endif @@});
@@ endfor @@
    @=name(variable)=@ := @@ if variable.type == 'Enumeration' @@Types.IntegerTo@= variable.declaredType.name =@(@@ endif @@FMI2Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@)@@ if variable.type == 'Enumeration' @@)@@ endif @@;
  else
    @=name(variable)=@ := outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ endblock @@