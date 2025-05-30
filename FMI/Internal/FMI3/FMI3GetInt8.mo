within FMI.Internal.FMI3;
impure function FMI3GetInt8
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Int8 values[nValues];

  external"C" FMU_FMI3GetInt8(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetInt8;
