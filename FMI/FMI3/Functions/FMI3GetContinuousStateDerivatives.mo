within FMI.FMI3.Functions;
impure function FMI3GetContinuousStateDerivatives
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU instance;
  input Integer nContinuousStates;
  output Real derivatives[nContinuousStates];
  external"C" FMU_FMI3GetContinuousStateDerivatives(instance, derivatives, nContinuousStates) annotation (Include="#include \"ModelicaFMI.h\"");
end FMI3GetContinuousStateDerivatives;
