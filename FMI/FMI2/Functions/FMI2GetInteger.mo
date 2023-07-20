within FMI.FMI2.Functions;
impure function FMI2GetInteger
  input Internal.ExternalFMU externalFMU;
    input Integer vr[nvr];
    input Integer nvr;
    output Integer value[nvr];
    external"C" FMU_FMI2GetInteger(externalFMU, vr, nvr, value) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI2GetInteger;
