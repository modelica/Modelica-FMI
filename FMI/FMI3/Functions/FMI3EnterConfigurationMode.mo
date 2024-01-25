within FMI.FMI3.Functions;
impure function FMI3EnterConfigurationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3EnterConfigurationMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3EnterConfigurationMode;
