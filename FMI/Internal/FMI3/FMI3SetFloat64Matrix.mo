within FMI.Internal.FMI3;
impure function FMI3SetFloat64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3Float64 values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetFloat64(externalFMU, valueReferences, 1, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3SetFloat64Matrix;
