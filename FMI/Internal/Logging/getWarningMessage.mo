within FMI.Internal.Logging;
impure function getWarningMessage
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  output String message;

  external"C" message = FMU_getWarningMessage(instance) annotation (Library={"ModelicaFMI"});

end getWarningMessage;
