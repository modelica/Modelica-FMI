within FMI.FMI3.Functions;
impure function FMI3GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3String values[nValues];

algorithm

  values := FMI.Internal.FMI3.FMI3GetString(instance, valueReference, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetString;
