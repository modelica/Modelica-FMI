within FMI.Internal.FMI3;
impure function FMI3Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3@=variable_type=@ values[nValues];

  external"C" FMU_FMI3Get@=variable_type=@(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3Get@=variable_type=@;

