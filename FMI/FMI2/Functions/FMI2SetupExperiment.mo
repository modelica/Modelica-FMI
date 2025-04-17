within FMI.FMI2.Functions;
impure function FMI2SetupExperiment
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Boolean toleranceDefined;
    input Real tolerance;
    input Real startTime;
    input Boolean stopTimeDefined;
    input Real stopTime;
algorithm
  FMI.Internal.FMI2.FMI2SetupExperiment(
    externalFMU,
    toleranceDefined,
    tolerance,
    startTime,
    stopTimeDefined,
    stopTime);
  FMI.Internal.Logging.logMessages(externalFMU);
end FMI2SetupExperiment;
