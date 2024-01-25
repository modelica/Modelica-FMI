within FMI.FMI3.Functions;
impure function FMI3ExitConfigurationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3ExitConfigurationMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3ExitConfigurationMode;
