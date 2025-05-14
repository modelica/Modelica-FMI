within FMI.FMI3.Functions;
impure function FMI3GetUInt8
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3UInt8 values[nValues];

algorithm

  values := FMI.Internal.FMI3.FMI3GetUInt8(instance, valueReference, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3GetUInt8;
