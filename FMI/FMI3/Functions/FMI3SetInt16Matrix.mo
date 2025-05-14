within FMI.FMI3.Functions;
impure function FMI3SetInt16Matrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3Int16 values[:,:];
  input Integer nValues;

algorithm

  FMI.Internal.FMI3.FMI3SetInt16Matrix(instance, valueReferences, values, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3SetInt16Matrix;
