within FMI.Internal.FMI3;
impure function FMI3SetUInt8Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3UInt8 values[:,:];
  input Integer nValues;

  external"C" FMU_FMI3SetUInt8(externalFMU, valueReferences, 1, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3SetUInt8Matrix;
