@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;
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
    @=fmi_type(variable, True)=@ @=name(variable)=@@=subscripts(variable)=@;
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
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values={@=name(variable)=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@=name(variable)=@);
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
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values={@=name(variable, '_start')=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=@=name(variable, '_start')=@);
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
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values={pre(@=name(variable)=@)});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, valueReferences={@=variable.valueReference=@}, values=pre(@=name(variable)=@));
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
@@ for variable in outputs @@
@@ if not variable.dimensions @@
      outputVariables.@=name(variable)=@ := scalar(FMI3Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@, nValues=1));
@@ elif variable.dimensions|length == 1 @@
      outputVariables.@=name(variable)=@ := FMI3Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@, nValues=@=numel(variable)=@);
@@ else @@
      outputVariables.@=name(variable)=@ := FMI3Get@=fmi_type(variable)=@Matrix(instance, @=variable.valueReference=@, m=size(@=name(variable)=@, 1), n=size(@=name(variable)=@, 2), nValues=@=numel(variable)=@);
@@ endif @@
@@ endfor @@
    end if;

  end when;

equation
@@ for variable in continuousOutputs @@

  if initial() then
@@ if not variable.dimensions @@
    @=name(variable)=@ = scalar(FMI3GetInitial@=fmi_type(variable)=@(instance, startTime, @=dependencies3(variable, 'Float64')=@@=dependencies3(variable, 'Int32')=@@=dependencies3(variable, 'Boolean')=@valueReference=@=variable.valueReference=@, nValues=1));
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ = FMI3GetInitial@=fmi_type(variable)=@(instance, startTime, @=dependencies3(variable, 'Float64')=@@=dependencies3(variable, 'Int32')=@@=dependencies3(variable, 'Boolean')=@valueReference=@=variable.valueReference=@, nValues=size(@=name(variable)=@, 1));
@@ else @@
    @=name(variable)=@ = FMI3GetInitial@=fmi_type(variable)=@Matrix(instance, startTime, @=dependencies3(variable, 'Float64')=@@=dependencies3(variable, 'Int32')=@@=dependencies3(variable, 'Boolean')=@valueReference=@=variable.valueReference=@, m=size(@=name(variable)=@, 1), n=size(@=name(variable)=@, 2), nValues=product(size(@=name(variable)=@)));
@@ endif @@
  else
    @=name(variable)=@ = outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ for variable in discreteOutputs @@

algorithm

  if initial() then
@@ for dependency in dependencies2(variable) @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(dependency)=@(instance, {@=dependency.valueReference=@}, {@=name(dependency)=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(dependency)=@(instance, {@=dependency.valueReference=@}, @=name(dependency)=@));
@@ else @@
    FMI3Set@=fmi_type(dependency)=@Matrix(instance, {@=dependency.valueReference=@}, @=name(dependency)=@, product(size(@=name(dependency)=@)));
@@ endif @@
@@ endfor @@
@@ if not variable.dimensions @@
    @=name(variable)=@ := scalar(FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, 1));
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ := FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, @=numel(variable)=@);
@@ else @@
    @=name(variable)=@ := FMI3Get@=fmi_type(variable)=@Matrix(instance, @=variable.valueReference=@, size(@=name(variable)=@, 1), size(@=name(variable)=@, 2), product(size(@=name(variable)=@)));
@@ endif @@
  else
    @=name(variable)=@ := outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ endblock @@