within FMI.FMI3.Functions;
impure function FMI3GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3String values[nValues];

  external"C" FMU_FMI3GetString(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3GetString;
