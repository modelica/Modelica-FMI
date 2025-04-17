within FMI.FMI3.Functions;
impure function FMI3EnterInitializationMode
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Boolean toleranceDefined;
  input Real tolerance;
  input Real startTime;
  input Boolean stopTimeDefined;
  input Real stopTime;

algorithm

  FMI.Internal.FMI3.FMI3EnterInitializationMode(
    instance,
    toleranceDefined,
    tolerance,
    startTime,
    stopTimeDefined,
    stopTime);

  FMI.Internal.Logging.logMessages(instance);

end FMI3EnterInitializationMode;
