within FMI.FMI2.Functions;
impure function FMI2GetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2Boolean value;

  external"C" FMU_FMI2GetBoolean(instance, valueReference, value) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2GetBoolean;
