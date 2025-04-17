within FMI.FMI2.Functions;
impure function FMI2SetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2Boolean values[nValues];

algorithm

  FMI.Internal.FMI2.FMI2SetBoolean(instance, valueReferences, nValues, values);

  FMI.Internal.Logging.logMessages(instance);

end FMI2SetBoolean;
