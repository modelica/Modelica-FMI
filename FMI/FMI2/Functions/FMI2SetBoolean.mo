within FMI.FMI2.Functions;
impure function FMI2SetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2Boolean values[nValues];

  external"C" FMU_FMI2SetBoolean(externalFMU, valueReferences, nValues, values) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2SetBoolean;
