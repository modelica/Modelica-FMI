within FMI.FMI2.Functions;
impure function FMI2SetInteger
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2Integer values[nValues];

  external"C" FMU_FMI2SetInteger(externalFMU, valueReferences, nValues, values) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2SetInteger;
