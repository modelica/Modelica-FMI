within FMI.Internal.FMI3;
impure function FMI3Get@=variable_type=@Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3@=variable_type=@ values[m, n];

  external"C" FMU_FMI3Get@=variable_type=@(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3Get@=variable_type=@Matrix;

