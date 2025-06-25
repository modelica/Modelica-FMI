within FMI.FMI3.Functions;
impure function FMI3Set@=variable_type=@Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3@=variable_type=@ values[:,:];
  input Integer nValues;

algorithm

  FMI.Internal.FMI3.FMI3Set@=variable_type=@Matrix(instance, valueReferences, values, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3Set@=variable_type=@Matrix;

