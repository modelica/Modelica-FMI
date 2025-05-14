within FMI.Internal.FMI3;
impure function FMI3GetUInt32Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt32 values[m, n];

  external"C" FMU_FMI3GetUInt32(externalFMU, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetUInt32Matrix;
