within FMI.Internal;
class ExternalFMU
  extends ExternalObject;

  impure function constructor
      output ExternalFMU externalFMU;
  external"C" externalFMU = FMU_Create() annotation (Library={"ModelicaFMI"});
  end constructor;

  impure function destructor
    input ExternalFMU externalFMU;
  external"C" FMU_Free(externalFMU) annotation (Library={"ModelicaFMI"});
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
