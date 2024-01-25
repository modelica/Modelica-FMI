within FMI.FMI3.Functions;
impure function FMI3UpdateDiscreteStates
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    output Boolean valuesOfContinuousStatesChanged;
    output Real nextEventTime;
    external"C" FMU_FMI3UpdateDiscreteStates(externalFMU, valuesOfContinuousStatesChanged, nextEventTime) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3UpdateDiscreteStates;
