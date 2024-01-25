within FMI.FMI3.Functions;
impure function FMI3SetBooleanMatrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3Boolean values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetBoolean(externalFMU, valueReferences, 1, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3SetBooleanMatrix;
