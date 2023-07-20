within FMI.Internal.Logging;
impure function getInfoMessage
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU instance;
  output String message;

  external"C" message = FMU_getInfoMessage(instance) annotation (Library={"ModelicaFMI"});

end getInfoMessage;
