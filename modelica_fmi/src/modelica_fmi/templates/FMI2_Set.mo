within FMI.FMI2.Functions;
impure function FMI2Set@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2@=variable_type=@ values[nValues];

algorithm

  FMI.Internal.FMI2.FMI2Set@=variable_type=@(instance, valueReferences, nValues, values);

  FMI.Internal.Logging.logMessages(instance);

end FMI2Set@=variable_type=@;

