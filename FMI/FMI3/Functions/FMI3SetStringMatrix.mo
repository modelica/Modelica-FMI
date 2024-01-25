within FMI.FMI3.Functions;
impure function FMI3SetStringMatrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3String values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetString(externalFMU, valueReferences, 1, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3SetStringMatrix;
