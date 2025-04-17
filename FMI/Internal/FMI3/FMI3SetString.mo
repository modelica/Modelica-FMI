within FMI.Internal.FMI3;
impure function FMI3SetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[nValueReferences];
  input Integer nValueReferences;
  input FMI.FMI3.Types.FMI3String values[nValues];
  input Integer nValues;

  external"C" FMU_FMI3SetString(externalFMU, valueReferences, nValueReferences, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3SetString;
