within FMI.FMI3.Functions;
impure function FMI3SetTime
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Real t;
    external"C" FMU_FMI3SetTime(externalFMU, t) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3SetTime;
