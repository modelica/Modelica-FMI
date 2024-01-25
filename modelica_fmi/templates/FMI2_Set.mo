within FMI.FMI2.Functions;
impure function FMI2Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2@=variable_type=@ values[nValues];

  external"C" FMU_FMI2Set@=variable_type=@(externalFMU, valueReferences, nValues, values) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2Set@=variable_type=@;

