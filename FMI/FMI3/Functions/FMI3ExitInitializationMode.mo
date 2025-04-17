within FMI.FMI3.Functions;
impure function FMI3ExitInitializationMode
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;

algorithm

  FMI.Internal.FMI3.FMI3ExitInitializationMode(instance);

  FMI.Internal.Logging.logMessages(instance);

end FMI3ExitInitializationMode;
