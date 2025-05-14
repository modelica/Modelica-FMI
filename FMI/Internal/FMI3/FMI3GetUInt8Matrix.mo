within FMI.Internal.FMI3;
impure function FMI3GetUInt8Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt8 values[m, n];

  external"C" FMU_FMI3GetUInt8(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetUInt8Matrix;
