within FMI.Internal.FMI3;
impure function FMI3GetString
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3String values[nValues];

  external"C" FMU_FMI3GetString(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetString;
