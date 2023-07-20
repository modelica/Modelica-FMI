within FMI.Internal.FMI2;
impure function FMI2Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2@=variable_type=@ value;

  external"C" FMU_FMI2Get@=variable_type=@(instance, valueReference, value) annotation (Library={"ModelicaFMI"});

end FMI2Get@=variable_type=@;

