within FMI.Internal.FMI2;
impure function FMI2SetupExperiment
  extends Modelica.Icons.Function;
  input Internal.ExternalFMU externalFMU;
    input Boolean toleranceDefined;
    input Real tolerance;
    input Real startTime;
    input Boolean stopTimeDefined;
    input Real stopTime;
    external"C" FMU_FMI2SetupExperiment(externalFMU, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime) annotation (Library={"ModelicaFMI"});
end FMI2SetupExperiment;
