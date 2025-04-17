within FMI.Internal.FMI2;
impure function FMI2GetReal
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2Real value;

  external"C" FMU_FMI2GetReal(instance, valueReference, value) annotation (Library={"ModelicaFMI"});

end FMI2GetReal;
