within FMI.Internal.Logging;
impure function getErrorMessage
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  output String message;

  external"C" message = FMU_getErrorMessage(instance) annotation (Library={"ModelicaFMI"});

end getErrorMessage;
