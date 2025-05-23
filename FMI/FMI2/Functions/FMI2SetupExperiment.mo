within FMI.FMI2.Functions;
impure function FMI2SetupExperiment
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU instance;
    input Boolean toleranceDefined;
    input Real tolerance;
    input Real startTime;
    input Boolean stopTimeDefined;
    input Real stopTime;
algorithm
  FMI.Internal.FMI2.FMI2SetupExperiment(
    instance,
    toleranceDefined,
    tolerance,
    startTime,
    stopTimeDefined,
    stopTime);
  FMI.Internal.Logging.logMessages(instance);
end FMI2SetupExperiment;
