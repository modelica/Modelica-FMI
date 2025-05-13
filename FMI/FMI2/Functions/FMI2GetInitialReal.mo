within FMI.FMI2.Functions;
impure function FMI2GetInitialReal
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;

  input Real _time;

  input Integer realInputValueReferences[:] = fill(0, 0);
  input Real realInputValues[:] = fill(0.0, 0);

  input Integer integerInputValueReferences[:] = fill(0, 0);
  input Integer integerInputValues[:] = fill(0, 0);

  input Integer booleanInputValueReferences[:] = fill(0, 0);
  input Boolean booleanInputValues[:] = fill(false, 0);

  input Integer valueReference;

  output Real value;

algorithm

  if size(realInputValueReferences, 1) > 0 then
    FMI2SetReal(instance, realInputValueReferences, size(realInputValueReferences, 1), realInputValues);
  end if;

  if size(integerInputValueReferences, 1) > 0 then
    FMI2SetInteger(instance, integerInputValueReferences, size(integerInputValueReferences, 1), integerInputValues);
  end if;

  if size(booleanInputValueReferences, 1) > 0 then
    FMI2SetBoolean(instance, booleanInputValueReferences, size(booleanInputValueReferences, 1), booleanInputValues);
  end if;

  value := FMI2GetReal(instance, valueReference);
  FMI.Internal.Logging.logMessages(instance);

end FMI2GetInitialReal;
