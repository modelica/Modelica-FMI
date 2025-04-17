within FMI.FMI3.Functions;
impure function FMI3GetInt64
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int64 values[nValues];

algorithm

  values := FMI.Internal.FMI3.FMI3GetInt64(instance, valueReference, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetInt64;
