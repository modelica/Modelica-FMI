@@ extends "FMU3.mo" @@
@@ block inputs @@
@@ for variable in inputs @@

  @=fmi_type(variable, prefix=True, declared=True)=@Input @=name(variable)=@@=subscripts(variable)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@@=annotations[variable.name]=@;
@@ endfor @@
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

@=set_variables(parameters)=@

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

@=set_variables(inputs, indent=4)=@

    FMI3DoStep(instance,
        currentCommunicationPoint=time,
        communicationStepSize=communicationStepSize);

@=get_variables(outputs, indent=4)=@

  end when;
@@ endblock @@