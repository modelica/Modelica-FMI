within FMI.Internal.FMI3;
impure function FMI3ExitConfigurationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3ExitConfigurationMode(externalFMU) annotation (Library={"ModelicaFMI"});
end FMI3ExitConfigurationMode;
