within FMI.FMI3.Functions;
impure function FMI3DoStep
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  input Real currentCommunicationPoint;
  input Real communicationStepSize;

algorithm

  FMI.Internal.FMI3.FMI3DoStep(instance, currentCommunicationPoint,
    communicationStepSize);

  FMI.Internal.Logging.logMessages(instance);

end FMI3DoStep;
