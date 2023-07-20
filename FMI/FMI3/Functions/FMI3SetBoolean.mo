within FMI.FMI3.Functions;
impure function FMI3SetBoolean
  input Internal.ExternalFMU externalFMU;
    input Integer valueReferences[nValueReferences];
    input Integer nValueReferences;
    input Boolean values[nValueReferences];
    external"C" FMU_FMI3SetBoolean(externalFMU, valueReferences, nValueReferences, values) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3SetBoolean;
