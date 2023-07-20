within FMI.FMI3.Functions;
impure function FMI3GetUInt64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt64 values[m, n];

algorithm

  values := FMI.Internal.FMI3.FMI3GetUInt64Matrix(instance, valueReference, m, n, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetUInt64Matrix;
