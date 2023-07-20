within FMI.FMI3.Functions;
impure function FMI3Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[:];
  input FMI.FMI3.Types.FMI3@=variable_type=@ values[:];

algorithm

  FMI.Internal.FMI3.FMI3Set@=variable_type=@(instance, valueReferences, size(valueReferences, 1), values, size(values, 1));

  FMI.Internal.Logging.logMessages(instance);

end FMI3Set@=variable_type=@;

