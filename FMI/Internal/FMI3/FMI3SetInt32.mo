within FMI.Internal.FMI3;
impure function FMI3SetInt32
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[nValueReferences];
  input Integer nValueReferences;
  input FMI.FMI3.Types.FMI3Int32 values[nValues];
  input Integer nValues;

  external"C" FMU_FMI3SetInt32(externalFMU, valueReferences, nValueReferences, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3SetInt32;
