within FMI.FMI2.Functions;
impure function FMI2SetReal
  input Internal.ExternalFMU externalFMU;
    input Integer vr[nvr];
    input Integer nvr;
    input Real value[nvr];
    external"C" FMU_FMI2SetReal(externalFMU, vr, nvr, value) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2SetReal;
