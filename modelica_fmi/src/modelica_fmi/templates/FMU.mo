within @=package=@;

block @=modelName=@
@@ if description @@
  "@=description=@"
@@ endif @@
  extends FMI.Internal.FMU;

@@ block imports @@
@@ endblock @@

@@ if enumerationTypeDefinitions @@
  package Types
    extends Modelica.Icons.TypesPackage;
@@ for typeDefinition in enumerationTypeDefinitions @@

    type @= typeDefinition.name =@ = enumeration(
@@ for item in typeDefinition.items @@
      @= name(item) =@@@ if item.description @@ "@= item.description =@"@@ endif @@@@ if not loop.last @@, @@ endif @@

@@ endfor @@
    )@@ if typeDefinition.description @@ "@= typeDefinition.description =@"@@ endif @@;

    connector @= typeDefinition.name =@Input = input @= typeDefinition.name =@ "'input @= typeDefinition.name =@' as connector" annotation (
      defaultComponentName="u",
      Icon(graphics={Polygon(
        points={{-100,100},{100,0},{-100,-100},{-100,100}},
        lineColor={255,127,0},
        fillColor={255,127,0},
        fillPattern=FillPattern.Solid),
    Text(
      textColor={0,0,0},
      extent={{140,-20},{140,20}},
        textString="%name",
        horizontalAlignment=TextAlignment.Left)},
                                         coordinateSystem(
      extent={{-100,-100},{100,100}},
      preserveAspectRatio=true,
      initialScale=0.2)),
      Diagram(coordinateSystem(
      preserveAspectRatio=true,
      initialScale=0.2,
      extent={{-100,-100},{100,100}}), graphics={Polygon(
        points={{0,50},{100,0},{0,-50},{0,50}},
        lineColor={255,127,0},
        fillColor={255,127,0},
        fillPattern=FillPattern.Solid), Text(
        extent={{-10,85},{-10,60}},
        textColor={255,127,0},
        textString="%name")}));

    connector @= typeDefinition.name =@Output = output @= typeDefinition.name =@ "'output @= typeDefinition.name =@' as connector" annotation (
      defaultComponentName="y",
      Icon(coordinateSystem(
      preserveAspectRatio=true,
      extent={{-100,-100},{100,100}}), graphics={Polygon(
        points={{-100,100},{100,0},{-100,-100},{-100,100}},
        lineColor={255,127,0},
        fillColor={255,255,255},
        fillPattern=FillPattern.Solid),
    Text(
      textColor={0,0,0},
      extent={{-180,-40},{-180,40}},
        textString="%name",
        horizontalAlignment=TextAlignment.Right)}),
      Diagram(coordinateSystem(
      preserveAspectRatio=true,
      extent={{-100,-100},{100,100}}), graphics={Polygon(
        points={{-100,50},{0,0},{-100,-50},{-100,50}},
        lineColor={255,127,0},
        fillColor={255,255,255},
        fillPattern=FillPattern.Solid), Text(
        extent={{30,110},{30,60}},
        textColor={255,127,0},
        textString="%name")}));

@@ block types @@
@@ endblock @@
@@ endfor @@

  end Types;

@@ endif @@

  parameter Modelica.Units.SI.Time communicationStepSize = @=communicationStepSize=@ annotation(Dialog(tab="FMI", group="Parameters"));

@@ block parameters @@
@@ endblock @@
@@ for variable in parameters @@

  parameter @=fmi_type(variable, prefix=True)=@ @=name(variable)=@@@ if variable.dimensions @@@=subscripts(variable)=@@@ endif @@@=modifiers(variable)=@ = @=start_value(variable)=@@@ if variable.description @@ "@=variable.description=@"@@ endif @@@=choices(variable)=@;
@@ endfor @@
@@ for variable in inputs @@

  parameter @=fmi_type(variable, prefix=True, declared=True)=@ @=name(variable, '_start')=@@=subscripts(variable)=@@=modifiers(variable)=@ = @=start_value(variable)=@ annotation(Dialog(tab="Initial", group="Start Values"));
@@ endfor @@
@@ for variable in inputs @@

  @=fmi_type(variable, prefix=True, declared=True)=@Input @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable, start=True)=@ @=annotations[variable.name]=@;
@@ endfor @@
@@ for variable in outputs @@

  @=fmi_type(variable, prefix=True, declared=True)=@Output @=name(variable)=@@=subscripts(variable)=@@=modifiers(variable)=@ @=annotations[variable.name]=@;
@@ endfor @@
@@ block equations @@
@@ endblock @@

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{@=x0=@,@=y0=@}, {@=x1=@,@=y1=@}}),
      graphics={
        Text(extent={{@=x0=@,@=y1+10=@}, {@=x1=@,@=y1+50=@}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{@=x0=@,@=y0=@},{@=x1=@,@=y1=@}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{@=x0=@,@=y0=@}, {@=x1=@,@=y1=@}})),
    experiment(StopTime=@=stopTime=@),
    uses(FMI(version="@=libraryVersion=@")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://@=rootPackage=@/Resources/FMUs/@=hash=@/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end @=modelName=@;
