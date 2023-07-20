within FMI.Internal.FMI2;
impure function FMI2ExitInitializationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI2ExitInitializationMode(externalFMU) annotation (Library={"ModelicaFMI"});
end FMI2ExitInitializationMode;
