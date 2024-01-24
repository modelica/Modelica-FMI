within FMI.FMI3.Functions;
impure function FMI3GetFloat32
  input Internal.ExternalFMU externalFMU;
    input Integer valueReferences[nValueReferences];
    input Integer nValueReferences;
    output Real values[nValueReferences];
    external"C" FMU_FMI3GetFloat32(externalFMU, valueReferences, nValueReferences, values) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3GetFloat32;
