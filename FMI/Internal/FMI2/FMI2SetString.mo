within FMI.Internal.FMI2;
impure function FMI2SetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2String values[nValues];

  external"C" FMU_FMI2SetString(instance, valueReferences, nValues, values) annotation (Library={"ModelicaFMI"});

end FMI2SetString;
