within FMI.FMI3.Functions;
impure function FMI3GetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Boolean values[nValues];

algorithm

  values := FMI.Internal.FMI3.FMI3GetBoolean(instance, valueReference, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetBoolean;
