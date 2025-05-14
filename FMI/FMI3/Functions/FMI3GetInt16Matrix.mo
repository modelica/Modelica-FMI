within FMI.FMI3.Functions;
impure function FMI3GetInt16Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int16 values[m, n];

algorithm

  values := FMI.Internal.FMI3.FMI3GetInt16Matrix(instance, valueReference, m, n, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetInt16Matrix;
