within FMI.Internal.FMI2;
impure function FMI2SetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2Boolean values[nValues];

  external"C" FMU_FMI2SetBoolean(instance, valueReferences, nValues, values) annotation (Library={"ModelicaFMI"});

end FMI2SetBoolean;
