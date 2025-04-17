within FMI.FMI2.Functions;
impure function FMI2Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2@=variable_type=@ value;

algorithm

  value := FMI.Internal.FMI2.FMI2Get@=variable_type=@(instance, valueReference);

  FMI.Internal.Logging.logMessages(instance);

end FMI2Get@=variable_type=@;

