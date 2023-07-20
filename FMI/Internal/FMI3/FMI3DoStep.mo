within FMI.Internal.FMI3;
impure function FMI3DoStep
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Real currentCommunicationPoint;
    input Real communicationStepSize;
    external"C" FMU_FMI3DoStep(externalFMU, currentCommunicationPoint, communicationStepSize) annotation (Library={"ModelicaFMI"});
  annotation();
end FMI3DoStep;
