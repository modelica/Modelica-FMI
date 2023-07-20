within FMI.Internal.FMI2;
impure function FMI2DoStep
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Real currentCommunicationPoint;
    input Real communicationStepSize;
    input Boolean noSetFMUStatePriorToCurrentPoint;
    external"C" FMU_FMI2DoStep(externalFMU, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint) annotation (Library={"ModelicaFMI"});
end FMI2DoStep;
