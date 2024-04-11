within FMI.FMI3.Functions;
impure function FMI3SetUInt64Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3UInt64 values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetUInt64(externalFMU, valueReferences, 1, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3SetUInt64Matrix;
