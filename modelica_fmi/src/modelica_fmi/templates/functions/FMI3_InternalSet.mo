within FMI.Internal.FMI3;
impure function FMI3Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[nValueReferences];
  input Integer nValueReferences;
  input FMI.FMI3.Types.FMI3@=variable_type=@ values[nValues];
  input Integer nValues;

  external"C" FMU_FMI3Set@=variable_type=@(externalFMU, valueReferences, nValueReferences, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3Set@=variable_type=@;

