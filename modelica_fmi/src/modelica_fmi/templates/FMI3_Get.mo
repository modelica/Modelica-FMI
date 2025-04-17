within FMI.FMI3.Functions;
impure function FMI3Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3@=variable_type=@ values[nValues];

  external"C" FMU_FMI3Get@=variable_type=@(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3Get@=variable_type=@;

