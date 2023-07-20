within FMI.Internal.FMI3;
impure function FMI3GetBoolean
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Boolean values[nValues];

  external"C" FMU_FMI3GetBoolean(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetBoolean;
