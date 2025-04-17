within FMI.FMI3.Functions;
impure function FMI3SetStringMatrix
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReferences[1];
  input FMI.FMI3.Types.FMI3String values[:,:];
  input Integer nValues;

algorithm

  FMI.Internal.FMI3.FMI3SetStringMatrix(instance, valueReferences, values, nValues);

  FMI.Internal.Logging.logMessages(instance);

end FMI3SetStringMatrix;
