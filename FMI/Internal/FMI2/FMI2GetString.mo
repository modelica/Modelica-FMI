within FMI.Internal.FMI2;
impure function FMI2GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2String value;

  external"C" FMU_FMI2GetString(instance, valueReference, value) annotation (Library={"ModelicaFMI"});

end FMI2GetString;
