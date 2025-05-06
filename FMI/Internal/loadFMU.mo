within FMI.Internal;
impure function loadFMU

  input ExternalFMU instance;
  input String unzipdir;
  input Integer fmiVersion;
  input String modelIdentifier;
  input String instanceName;
  input Integer interfaceType;
  input String instantiationToken;
  input Boolean visible;
  input Boolean loggingOn;
  input Boolean logFMICalls;
  input Boolean logToFile;
  input String logFile;
  input Boolean copyPlatformBinary;

external"C" FMULoad(instance, unzipdir, fmiVersion, modelIdentifier, instanceName, interfaceType, instantiationToken, visible, loggingOn, logFMICalls, logToFile, logFile, copyPlatformBinary) annotation (Library={"ModelicaFMI"});

end loadFMU;
