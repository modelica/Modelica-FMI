within FMI;
package Internal
  extends Modelica.Icons.InternalPackage;

  class ExternalFMU
    extends ExternalObject;

    impure function constructor
        input String unzipdir;
        input Integer fmiVersion;
        input String modelIdentifier;
        input String instanceName;
        input Integer interfaceType;
        input String instantiationToken;
        input Boolean visible;
        input Boolean loggingOn;
        input Boolean logFMICalls;
        input Boolean logToFile;
        input String logFile;
        input Boolean copyPlatformBinary;
        output ExternalFMU externalFMU;
    external"C" externalFMU =
          FMU_load(unzipdir, fmiVersion, modelIdentifier, instanceName, interfaceType, instantiationToken, visible, loggingOn, logFMICalls, logToFile, logFile, copyPlatformBinary) annotation (Include="#include \"ModelicaFMI.h\"");

    end constructor;

    impure function destructor
      input ExternalFMU externalFMU;
    external"C" FMU_free(externalFMU) annotation (Include="#include \"ModelicaFMI.h\"");
    end destructor;

    annotation (Icon(coordinateSystem(preserveAspectRatio=false), graphics={
            Rectangle(
              lineColor={160,160,164},
              fillColor={160,160,164},
              fillPattern=FillPattern.Solid,
              extent={{-100,-100},{100,100}},
              radius=25.0)}),                                      Diagram(
          coordinateSystem(preserveAspectRatio=false)));
  end ExternalFMU;
end Internal;
