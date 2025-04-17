within FMI.FMI2.Functions;
impure function FMI2SetReal
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReferences[nValues];
  input Integer nValues;
  input FMI.FMI2.Types.FMI2Real values[nValues];

algorithm

  FMI.Internal.FMI2.FMI2SetReal(instance, valueReferences, nValues, values);

  FMI.Internal.Logging.logMessages(instance);

end FMI2SetReal;
