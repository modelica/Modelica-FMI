within FMI.Internal.Logging;
function logMessages
  extends Modelica.Icons.Function;

  input Internal.ExternalFMU externalFMU;

protected
  String message;

algorithm

  message := FMI.Internal.Logging.getInfoMessage(externalFMU);

  while not Modelica.Utilities.Strings.isEmpty(message) loop
    Modelica.Utilities.Streams.print(message);
    message := FMI.Internal.Logging.getInfoMessage(externalFMU);
  end while;

  message := FMI.Internal.Logging.getWarningMessage(externalFMU);

  while not Modelica.Utilities.Strings.isEmpty(message) loop
    assert(false, message, AssertionLevel.warning);
    message := FMI.Internal.Logging.getWarningMessage(externalFMU);
  end while;

  message := FMI.Internal.Logging.getErrorMessage(externalFMU);

  assert(Modelica.Utilities.Strings.isEmpty(message), message, AssertionLevel.error);

end logMessages;
