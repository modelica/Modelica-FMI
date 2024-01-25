within FMI.FMI3.Functions;
impure function FMI3SetInt64
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;
  input FMI.FMI3.Types.FMI3ValueReference valueReferences[:];
  input FMI.FMI3.Types.FMI3Int64 values[:];

  external"C" FMU_FMI3SetInt64(externalFMU, valueReferences, size(valueReferences, 1), values, size(values, 1)) annotation (Include="#include \"ModelicaFMI.h\"");

end FMI3SetInt64;
