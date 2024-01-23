within FMI.FMI3.Functions;
impure function FMI3SetFloat32
  input Internal.ExternalFMU externalFMU;
    input Integer valueReferences[nValueReferences];
    input Integer nValueReferences;
    input Real values[nValueReferences];
    external"C" FMU_FMI3SetFloat32(externalFMU, valueReferences, nValueReferences, values) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3SetFloat32;
