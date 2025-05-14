within FMI.Internal.FMI3;
impure function FMI3SetUInt16Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3UInt16 values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetUInt16(externalFMU, valueReferences, 1, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3SetUInt16Matrix;
