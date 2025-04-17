within FMI.FMI2.Functions;
impure function FMI2DoStep
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Real currentCommunicationPoint;
    input Real communicationStepSize;
    input Boolean noSetFMUStatePriorToCurrentPoint;
algorithm
  FMI.Internal.FMI2.FMI2DoStep(
    externalFMU,
    currentCommunicationPoint,
    communicationStepSize,
    noSetFMUStatePriorToCurrentPoint);
  FMI.Internal.Logging.logMessages(externalFMU);
end FMI2DoStep;
