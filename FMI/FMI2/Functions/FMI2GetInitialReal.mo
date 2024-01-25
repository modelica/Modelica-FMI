within FMI.FMI2.Functions;
impure function FMI2GetInitialReal
  input Internal.ExternalFMU instance;
  input Real _time;
  input Integer realInputVRs[:];
  input Real realInputValues[:];
  input Integer integerInputVRs[:];
  input Integer integerInputValues[:];
  input Integer booleanInputVRs[:];
  input Boolean booleanInputValues[:];
  input Integer valueReference;
  output Real value;
algorithm
  FMI2SetReal(instance, realInputVRs, size(realInputVRs, 1), realInputValues);
  FMI2SetInteger(instance, integerInputVRs, size(integerInputVRs, 1), integerInputValues);
  FMI2SetBoolean(instance, booleanInputVRs, size(booleanInputVRs, 1), booleanInputValues);
  value := FMI2GetReal(instance, valueReference);
end FMI2GetInitialReal;
