within FMI.FMI3.Functions;
impure function FMI3GetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Boolean values[nValues];

  external"C" FMU_FMI3GetBoolean(externalFMU, valueReference, values, nValues) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3GetBoolean;
