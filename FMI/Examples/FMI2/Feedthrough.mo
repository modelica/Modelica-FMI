within FMI.Examples.FMI2;

block Feedthrough
  "A model to test different variable types, causalities, and variabilities"
  extends FMI.Internal.FMU;

  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;

  package Types
    extends Modelica.Icons.TypesPackage;

    type Option = enumeration(
      Option_1 "First option", 
      Option_2 "Second option"
    );

    connector OptionInput = input Option "'input Option' as connector" annotation (
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

    connector OptionOutput = output Option "'output Option' as connector" annotation (
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

    function OptionToInteger

      input Option enumerationValue;

      output FMI2Integer integerValue;

    algorithm

      if enumerationValue == Option.Option_1 then
        integerValue := 1;
      elseif enumerationValue == Option.Option_2 then
        integerValue := 2;
      else
        assert(false, "Illegal value");
      end if;

    end OptionToInteger;

    function IntegerToOption

      input FMI2Integer integerValue;

      output Option enumerationValue;

    algorithm

      if integerValue == 1 then
        enumerationValue := Option.Option_1;
      elseif integerValue == 2 then
        enumerationValue := Option.Option_2;
      else
        assert(false, "Illegal value");
      end if;

    end IntegerToOption;

  end Types;


  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI2Real Float64_fixed_parameter = 0;

  parameter FMI2Real Float64_tunable_parameter = 0;

  parameter FMI2String String_parameter = "Set me!";

  parameter FMI2Real Float64_continuous_input_start = 0 annotation(Dialog(tab="Initial", group="Start Values"));

  parameter FMI2Real Float64_discrete_input_start = 0 annotation(Dialog(tab="Initial", group="Start Values"));

  parameter FMI2Integer Int32_input_start = 0 annotation(Dialog(tab="Initial", group="Start Values"));

  parameter FMI2Boolean Boolean_input_start = false annotation(Dialog(tab="Initial", group="Start Values"));

  parameter Types.Option Enumeration_input_start = Types.Option.Option_1 annotation(Dialog(tab="Initial", group="Start Values"));

  FMI2RealInput Float64_continuous_input(start=Float64_continuous_input_start) annotation (Placement(transformation(extent={ { -640, 146.66666666666669 }, { -600, 186.66666666666669 } }), iconTransformation(extent={ { -640, 146.66666666666669 }, { -600, 186.66666666666669 } })));

  FMI2RealInput Float64_discrete_input(start=Float64_discrete_input_start) annotation (Placement(transformation(extent={ { -640, 63.33333333333334 }, { -600, 103.33333333333334 } }), iconTransformation(extent={ { -640, 63.33333333333334 }, { -600, 103.33333333333334 } })));

  FMI2IntegerInput Int32_input(start=Int32_input_start) annotation (Placement(transformation(extent={ { -640, -20.0 }, { -600, 20.0 } }), iconTransformation(extent={ { -640, -20.0 }, { -600, 20.0 } })));

  FMI2BooleanInput Boolean_input(start=Boolean_input_start) annotation (Placement(transformation(extent={ { -640, -103.33333333333331 }, { -600, -63.333333333333314 } }), iconTransformation(extent={ { -640, -103.33333333333331 }, { -600, -63.333333333333314 } })));

  Types.OptionInput Enumeration_input(start=Enumeration_input_start) annotation (Placement(transformation(extent={ { -640, -186.66666666666663 }, { -600, -146.66666666666663 } }), iconTransformation(extent={ { -640, -186.66666666666663 }, { -600, -146.66666666666663 } })));

  FMI2RealOutput Float64_continuous_output annotation (Placement(transformation(extent={ { 600, 156.66666666666669 }, { 620, 176.66666666666669 } }), iconTransformation(extent={ { 600, 156.66666666666669 }, { 620, 176.66666666666669 } })));

  FMI2RealOutput Float64_discrete_output annotation (Placement(transformation(extent={ { 600, 73.33333333333334 }, { 620, 93.33333333333334 } }), iconTransformation(extent={ { 600, 73.33333333333334 }, { 620, 93.33333333333334 } })));

  FMI2IntegerOutput Int32_output annotation (Placement(transformation(extent={ { 600, -10.0 }, { 620, 10.0 } }), iconTransformation(extent={ { 600, -10.0 }, { 620, 10.0 } })));

  FMI2BooleanOutput Boolean_output annotation (Placement(transformation(extent={ { 600, -93.33333333333331 }, { 620, -73.33333333333331 } }), iconTransformation(extent={ { 600, -93.33333333333331 }, { 620, -73.33333333333331 } })));

  Types.OptionOutput Enumeration_output annotation (Placement(transformation(extent={ { 600, -176.66666666666663 }, { 620, -156.66666666666663 } }), iconTransformation(extent={ { 600, -176.66666666666663 }, { 620, -156.66666666666663 } })));

protected

  record OutputVariables
    Real Float64_continuous_output;
    Real Float64_discrete_output;
    Integer Int32_output;
    Boolean Boolean_output;
    Types.Option Enumeration_output;
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/ea83733"),
    fmiVersion=2,
    modelIdentifier="Feedthrough",
    instanceName=getInstanceName(),
    interfaceType=1,
    instantiationToken="{37B954F1-CC86-4D8F-B97F-C7C36F6670D2}",
    visible=visible,
    loggingOn=loggingOn,
    logFMICalls=logFMICalls,
    logToFile=logToFile,
    logFile=logFile,
    copyPlatformBinary=false);

  FMI.Internal.Logging.logMessages(instance);

  if not startValuesSet then
    startTime := time;
    FMI2SetReal(instance, valueReferences={5}, nValues=1, values={Float64_fixed_parameter});
    FMI2SetReal(instance, valueReferences={6}, nValues=1, values={Float64_tunable_parameter});
    FMI2SetString(instance, valueReferences={29}, nValues=1, values={String_parameter});
    FMI2SetupExperiment(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);
    FMI2EnterInitializationMode(instance);
    FMI2SetReal(instance, valueReferences={7}, nValues=1, values={Float64_continuous_input_start});
    FMI2SetReal(instance, valueReferences={9}, nValues=1, values={Float64_discrete_input_start});
    FMI2SetInteger(instance, valueReferences={19}, nValues=1, values={Int32_input_start});
    FMI2SetBoolean(instance, valueReferences={27}, nValues=1, values={Boolean_input_start});
    FMI2SetInteger(instance, valueReferences={32}, nValues=1, values={Types.OptionToInteger(Enumeration_input_start)});
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

    FMI2SetReal(instance, valueReferences={7}, nValues=1, values={pre(Float64_continuous_input)});
    FMI2SetReal(instance, valueReferences={9}, nValues=1, values={pre(Float64_discrete_input)});
    FMI2SetInteger(instance, valueReferences={19}, nValues=1, values={pre(Int32_input)});
    FMI2SetBoolean(instance, valueReferences={27}, nValues=1, values={pre(Boolean_input)});
    FMI2SetInteger(instance, valueReferences={32}, nValues=1, values={Types.OptionToInteger(pre(Enumeration_input))});

    if not initialized and not initial() then
      FMI2ExitInitializationMode(instance);
      initialized := true;
    end if;

    if time >= startTime + communicationStepSize then
      FMI2DoStep(instance,
        currentCommunicationPoint=time - communicationStepSize,
        communicationStepSize=communicationStepSize,
        noSetFMUStatePriorToCurrentPoint=true);
    end if;

    if not initial() then
      outputVariables.Float64_continuous_output := FMI2GetReal(instance, valueReference=8);
      outputVariables.Float64_discrete_output := FMI2GetReal(instance, valueReference=10);
      outputVariables.Int32_output := FMI2GetInteger(instance, valueReference=20);
      outputVariables.Boolean_output := FMI2GetBoolean(instance, valueReference=28);
      outputVariables.Enumeration_output := Types.IntegerToOption(FMI2GetInteger(instance, valueReference=33));
    end if;

  end when;

equation

  if initial() then
    Float64_continuous_output = pure(FMI2GetInitialReal(instance, startTime, realInputValueReferences={7}, realInputValues={Float64_continuous_input}, valueReference=8));
  else
    Float64_continuous_output = outputVariables.Float64_continuous_output;
  end if;

algorithm

  if initial() then
    FMI2SetReal(instance, valueReferences={9}, nValues=1, values={Float64_discrete_input});
    Float64_discrete_output := FMI2GetReal(instance, valueReference=10);
  else
    Float64_discrete_output := outputVariables.Float64_discrete_output;
  end if;

algorithm

  if initial() then
    FMI2SetInteger(instance, valueReferences={19}, nValues=1, values={Int32_input});
    Int32_output := FMI2GetInteger(instance, valueReference=20);
  else
    Int32_output := outputVariables.Int32_output;
  end if;

algorithm

  if initial() then
    FMI2SetBoolean(instance, valueReferences={27}, nValues=1, values={Boolean_input});
    FMI2SetString(instance, valueReferences={29}, nValues=1, values={String_parameter});
    Boolean_output := FMI2GetBoolean(instance, valueReference=28);
  else
    Boolean_output := outputVariables.Boolean_output;
  end if;

algorithm

  if initial() then
    FMI2SetInteger(instance, valueReferences={32}, nValues=1, values={Types.OptionToInteger(Enumeration_input)});
    Enumeration_output := Types.IntegerToOption(FMI2GetInteger(instance, valueReference=33));
  else
    Enumeration_output := outputVariables.Enumeration_output;
  end if;

  annotation (
    Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-600,-250}, {600,250}}),
      graphics={
        Text(extent={{-600,260}, {600,300}}, lineColor={0,0,255}, textString="%name"),
        Rectangle(extent={{-600,-250},{600,250}}, lineColor={95,95,95}, fillColor={255,255,255}, fillPattern=FillPattern.Solid)
      }
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-600,-250}, {600,250}})),
    experiment(StopTime=2.0),
    uses(FMI(version="0.0.6")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/ea83733/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Feedthrough;