within FMI.Internal.FMI3;
impure function FMI3GetFloat64
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Integer valueReference;
  input Integer nValues;

  output FMI.FMI3.Types.FMI3Float64 values[nValues];

  external"C" FMU_FMI3GetFloat64(instance, valueReference, values, nValues) annotation (Library={"ModelicaFMI"});

end FMI3GetFloat64;
