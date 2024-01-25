within FMI.FMI2.Functions;
impure function FMI2GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2String value;

  external"C" FMU_FMI2GetString(instance, valueReference, value) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2GetString;
