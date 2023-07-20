within FMI.Internal.FMI3;
impure function FMI3EnterConfigurationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3EnterConfigurationMode(externalFMU) annotation (Library={"ModelicaFMI"});
end FMI3EnterConfigurationMode;
