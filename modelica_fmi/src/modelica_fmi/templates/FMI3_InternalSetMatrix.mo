within FMI.Internal.FMI3;
impure function FMI3Set@=variable_type=@Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3@=variable_type=@ values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3Set@=variable_type=@(externalFMU, valueReferences, 1, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3Set@=variable_type=@Matrix;

