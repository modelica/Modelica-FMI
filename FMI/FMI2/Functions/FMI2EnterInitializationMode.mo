within FMI.FMI2.Functions;
impure function FMI2EnterInitializationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
algorithm
  FMI.Internal.FMI2.FMI2EnterInitializationMode(externalFMU);
  FMI.Internal.Logging.logMessages(externalFMU);
end FMI2EnterInitializationMode;
