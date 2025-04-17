within FMI.FMI3.Functions;
impure function FMI3Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[:];
  input FMI.FMI3.Types.FMI3@=variable_type=@ values[:];

  external"C" FMU_FMI3Set@=variable_type=@(externalFMU, valueReferences, size(valueReferences, 1), values, size(values, 1)) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3Set@=variable_type=@;

