within FMI.FMI3.Functions;
impure function FMI3SetInt32Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3Int32 values[:,:];
  input Integer nValues;

algorithm

  FMI.Internal.FMI3.FMI3SetInt32Matrix(instance, valueReferences, values, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3SetInt32Matrix;
