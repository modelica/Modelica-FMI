within FMI.FMI2.Functions;
impure function FMI2GetReal
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2Real value;

algorithm

  value := FMI.Internal.FMI2.FMI2GetReal(instance, valueReference);

  FMI.Internal.Logging.logMessages(instance);

end FMI2GetReal;
