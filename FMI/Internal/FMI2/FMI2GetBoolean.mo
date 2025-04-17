within FMI.Internal.FMI2;
impure function FMI2GetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2Boolean value;

  external"C" FMU_FMI2GetBoolean(instance, valueReference, value) annotation (Library={"ModelicaFMI"});

end FMI2GetBoolean;
