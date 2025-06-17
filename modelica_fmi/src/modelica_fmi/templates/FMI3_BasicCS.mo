@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;
@@ endblock @@
@@ block equations @@

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

@= set_variables(parameters) =@

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

@= set_variables(inputs, indent=4) =@

    FMI3DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize);

@@ for variable in outputs @@
@@ if not variable.dimensions @@
    @=name(variable)=@ := scalar(@@ if variable.type == 'Enumeration' @@Types.Int64To@=variable.declaredType.name=@(@@ endif @@FMI3Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@, nValues=1)@@ if variable.type == 'Enumeration' @@)@@ endif @@);
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ := @@ if variable.type == 'Enumeration' @@Types.Int64To@=variable.declaredType.name=@(@@ endif @@FMI3Get@=fmi_type(variable)=@(instance, valueReference=@=variable.valueReference=@, nValues=@=numel(variable)=@)@@ if variable.type == 'Enumeration' @@)@@ endif @@;
@@ else @@
    @=name(variable)=@ := FMI3Get@=fmi_type(variable)=@Matrix(instance, @=variable.valueReference=@, m=size(@=name(variable)=@, 1), n=size(@=name(variable)=@, 2), nValues=@=numel(variable)=@);
@@ endif @@
@@ endfor @@

  end when;
@@ endblock @@