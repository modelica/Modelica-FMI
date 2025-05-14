within FMI.FMI3.Functions;
impure function FMI3SetInt8
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[:];
  input FMI.FMI3.Types.FMI3Int8 values[:];

algorithm

  FMI.Internal.FMI3.FMI3SetInt8(instance, valueReferences, size(valueReferences, 1), values, size(values, 1));

  FMI.Internal.Logging.logMessages(instance);

end FMI3SetInt8;
