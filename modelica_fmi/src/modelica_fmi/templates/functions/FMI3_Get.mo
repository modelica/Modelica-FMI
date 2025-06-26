within FMI.FMI3.Functions;
impure function FMI3Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3@=variable_type=@ values[nValues];

algorithm

  values := FMI.Internal.FMI3.FMI3Get@=variable_type=@(instance, valueReference, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3Get@=variable_type=@;

