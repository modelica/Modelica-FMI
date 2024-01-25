within FMI.FMI2.Functions;
impure function FMI2Get@=variable_type=@
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input FMI.FMI2.Types.FMI2ValueReference valueReference;

  output FMI.FMI2.Types.FMI2@=variable_type=@ value;

  external"C" FMU_FMI2Get@=variable_type=@(instance, valueReference, value) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI2Get@=variable_type=@;

