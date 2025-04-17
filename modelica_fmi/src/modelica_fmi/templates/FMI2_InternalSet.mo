within FMI.Internal.FMI2;
impure function FMI2Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2@=variable_type=@ values[nValues];

  external"C" FMU_FMI2Set@=variable_type=@(instance, valueReferences, nValues, values) annotation (Library={"ModelicaFMI"});

end FMI2Set@=variable_type=@;

