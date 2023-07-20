within FMI.FMI2.Functions;
impure function FMI2GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2String value;

algorithm

  value := FMI.Internal.FMI2.FMI2GetString(instance, valueReference);

  FMI.Internal.Logging.logMessages(instance);

end FMI2GetString;
