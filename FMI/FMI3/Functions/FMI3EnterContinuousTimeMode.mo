within FMI.FMI3.Functions;
impure function FMI3EnterContinuousTimeMode
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    external"C" FMU_FMI3EnterContinuousTimeMode(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3EnterContinuousTimeMode;
