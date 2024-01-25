within FMI.FMI3.Functions;
impure function FMI3Get@=variable_type=@Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3@=variable_type=@ values[m, n];

  external"C" FMU_FMI3Get@=variable_type=@(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3Get@=variable_type=@Matrix;

