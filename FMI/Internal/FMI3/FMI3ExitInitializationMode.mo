within FMI.Internal.FMI3;
impure function FMI3ExitInitializationMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3ExitInitializationMode(externalFMU) annotation (Library={"ModelicaFMI"});
end FMI3ExitInitializationMode;
