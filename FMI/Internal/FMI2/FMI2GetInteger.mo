within FMI.Internal.FMI2;
impure function FMI2GetInteger
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2Integer value;

  external"C" FMU_FMI2GetInteger(instance, valueReference, value) annotation (Library={"ModelicaFMI"});

end FMI2GetInteger;
