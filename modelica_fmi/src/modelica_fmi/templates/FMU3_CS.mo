@@ extends "FMU3.mo" @@
@@ block inputs @@
@@ for variable in inputs @@

  parameter @=fmi_type(variable, prefix=True, declared=True)=@ @=name(variable, '_start')=@@=subscripts(variable)=@@=modifiers(variable)=@ = @=start_value(variable)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@ annotation(Dialog(tab="Initial", group="Start Values"));
@@ endfor @@
@@ for variable in inputs @@

  @=fmi_type(variable, prefix=True, declared=True)=@Input @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable, start=True)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@@=annotations[variable.name]=@;
@@ endfor @@
@@ endblock @@
@@ block equations @@

protected

  parameter Boolean startValuesSet(start=false, fixed=false);

  Boolean initialized(start=false, fixed=true);

  record OutputVariables
@@ for variable in outputs @@
    @=fmi_type(variable, prefix=True, declared=True)=@ @=name(variable)=@@=subscripts(variable)=@;
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
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInt64(@@ endif @@{@=name(variable)=@}@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInt64(@@ endif @@@=name(variable)=@@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, valueReferences={@=variable.valueReference=@}, values=@=name(variable)=@, nValues=@=numel(variable)=@);
@@ endif @@
@@ endfor @@
    FMI3EnterInitializationMode(instance,
        toleranceDefined=tolerance > 0.0,
        tolerance=tolerance,
        startTime=startTime,
        stopTimeDefined=stopTime < Modelica.Constants.inf,
        stopTime=stopTime);
@@ for variable in inputs @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInt64(@@ endif @@{@=name(variable, '_start')=@}@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@= variable.declaredType.name =@ToInt64(@@ endif @@@=name(variable, '_start')=@@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, valueReferences={@=variable.valueReference=@}, values=@=name(variable, '_start')=@, nValues=@=numel(variable)=@);
@@ endif @@
@@ endfor @@
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

@@ for variable in inputs @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@=variable.declaredType.name=@ToInt64(@@ endif @@{pre(@=name(variable)=@)}@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@@ if variable.type == 'Enumeration' @@Types.@=variable.declaredType.name=@ToInt64(@@ endif @@pre(@=name(variable)=@)@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, valueReferences={@=variable.valueReference=@}, values=pre(@=name(variable)=@), nValues=@=numel(variable)=@);
@@ endif @@
@@ endfor @@

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
@=get_variables(outputs, indent=6, prefix="outputVariables.")=@
    end if;

  end when;

equation
@@ for variable in continuousOutputs @@

  if initial() then
@@ if not variable.dimensions @@
    @=name(variable)=@ = scalar(pure(FMI3GetInitial@=fmi_type(variable)=@(instance, startTime, @=dependencies3(variable, ['Float32', 'Float64', 'Int8', 'UInt8', 'Int16', 'UInt16', 'Int32', 'UInt32', 'Int64', 'UInt64', 'Boolean'])=@valueReference=@=variable.valueReference=@, nValues=1)));
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ = pure(FMI3GetInitial@=fmi_type(variable)=@(instance, startTime, @=dependencies3(variable, ['Float32', 'Float64', 'Int8', 'UInt8', 'Int16', 'UInt16', 'Int32', 'UInt32', 'Int64', 'UInt64', 'Boolean'])=@valueReference=@=variable.valueReference=@, nValues=size(@=name(variable)=@, 1)));
@@ else @@
    @=name(variable)=@ = pure(FMI3GetInitial@=fmi_type(variable)=@Matrix(instance, startTime, @=dependencies3(variable, ['Float32', 'Float64', 'Int8', 'UInt8', 'Int16', 'UInt16', 'Int32', 'UInt32', 'Int64', 'UInt64', 'Boolean'])=@valueReference=@=variable.valueReference=@, m=size(@=name(variable)=@, 1), n=size(@=name(variable)=@, 2), nValues=product(size(@=name(variable)=@))));
@@ endif @@
  else
    @=name(variable)=@ = outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ for variable in discreteOutputs @@

algorithm

  if initial() then
@=set_variables(dependencies2(variable), indent=4)=@
@=get_variables([variable], indent=4)=@
  else
    @=name(variable)=@ := outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ endblock @@