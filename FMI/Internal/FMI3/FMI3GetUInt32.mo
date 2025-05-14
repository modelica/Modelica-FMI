within FMI.Internal.FMI3;
impure function FMI3GetUInt32
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt32 values[nValues];

  external"C" FMU_FMI3GetUInt32(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetUInt32;
