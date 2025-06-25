within FMI.Examples.FMI3;

block Feedthrough
  "A model to test different variable types, causalities, and variabilities"
  extends FMI.Internal.FMU;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;

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
            fillPattern=FillPattern.Solid)}, coordinateSystem(
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
            fillPattern=FillPattern.Solid)}),
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

    function OptionToInt64

      input Option enumerationValues[:];

      output FMI3Int64 integerValues[size(enumerationValues, 1)];

    algorithm

      for i in 1:size(enumerationValues, 1) loop

        if enumerationValues[i] == Option.Option_1 then
          integerValues[i] := 1;
        elseif enumerationValues[i] == Option.Option_2 then
          integerValues[i] := 2;
        else
          assert(false, "Illegal value");
        end if;

      end for;

    end OptionToInt64;

    function Int64ToOption

      input FMI3Int64 integerValues[:];

      output Option enumerationValues[size(integerValues, 1)];

    algorithm

      for i in 1:size(integerValues, 1) loop

        if integerValues[i] == 1 then
          enumerationValues[i] := Option.Option_1;
        elseif integerValues[i] == 2 then
          enumerationValues[i] := Option.Option_2;
        else
          assert(false, "Illegal value");
        end if;

      end for;

    end Int64ToOption;

  end Types;


  parameter Modelica.Units.SI.Time communicationStepSize = 1e-2 annotation(Dialog(tab="FMI", group="Parameters"));

  parameter FMI3Float64 Float64_fixed_parameter = 0;

  parameter FMI3Float64 Float64_tunable_parameter = 0;

  parameter FMI3String String_parameter = "Set me!";

  FMI3Float32Input Float32_continuous_input annotation(Placement(transformation(extent={ { -140, 60.0 }, { -100, 100.0 } }), iconTransformation(extent={ { -140, 60.0 }, { -100, 100.0 } })));

  FMI3Float32Input Float32_discrete_input annotation(Placement(transformation(extent={ { -140, 47.69230769230769 }, { -100, 87.6923076923077 } }), iconTransformation(extent={ { -140, 47.69230769230769 }, { -100, 87.6923076923077 } })));

  FMI3Float64Input Float64_continuous_input annotation(Placement(transformation(extent={ { -140, 35.38461538461539 }, { -100, 75.38461538461539 } }), iconTransformation(extent={ { -140, 35.38461538461539 }, { -100, 75.38461538461539 } })));

  FMI3Float64Input Float64_discrete_input annotation(Placement(transformation(extent={ { -140, 23.076923076923073 }, { -100, 63.07692307692307 } }), iconTransformation(extent={ { -140, 23.076923076923073 }, { -100, 63.07692307692307 } })));

  FMI3Int8Input Int8_input annotation(Placement(transformation(extent={ { -140, 10.769230769230766 }, { -100, 50.76923076923077 } }), iconTransformation(extent={ { -140, 10.769230769230766 }, { -100, 50.76923076923077 } })));

  FMI3UInt8Input UInt8_input annotation(Placement(transformation(extent={ { -140, -1.53846153846154 }, { -100, 38.46153846153846 } }), iconTransformation(extent={ { -140, -1.53846153846154 }, { -100, 38.46153846153846 } })));

  FMI3Int16Input Int16_input annotation(Placement(transformation(extent={ { -140, -13.846153846153854 }, { -100, 26.153846153846146 } }), iconTransformation(extent={ { -140, -13.846153846153854 }, { -100, 26.153846153846146 } })));

  FMI3UInt16Input UInt16_input annotation(Placement(transformation(extent={ { -140, -26.15384615384616 }, { -100, 13.84615384615384 } }), iconTransformation(extent={ { -140, -26.15384615384616 }, { -100, 13.84615384615384 } })));

  FMI3Int32Input Int32_input annotation(Placement(transformation(extent={ { -140, -38.46153846153847 }, { -100, 1.538461538461533 } }), iconTransformation(extent={ { -140, -38.46153846153847 }, { -100, 1.538461538461533 } })));

  FMI3UInt32Input UInt32_input annotation(Placement(transformation(extent={ { -140, -50.769230769230774 }, { -100, -10.769230769230774 } }), iconTransformation(extent={ { -140, -50.769230769230774 }, { -100, -10.769230769230774 } })));

  FMI3Int64Input Int64_input annotation(Placement(transformation(extent={ { -140, -63.07692307692308 }, { -100, -23.07692307692308 } }), iconTransformation(extent={ { -140, -63.07692307692308 }, { -100, -23.07692307692308 } })));

  FMI3UInt64Input UInt64_input annotation(Placement(transformation(extent={ { -140, -75.38461538461539 }, { -100, -35.38461538461539 } }), iconTransformation(extent={ { -140, -75.38461538461539 }, { -100, -35.38461538461539 } })));

  FMI3BooleanInput Boolean_input annotation(Placement(transformation(extent={ { -140, -87.69230769230771 }, { -100, -47.69230769230771 } }), iconTransformation(extent={ { -140, -87.69230769230771 }, { -100, -47.69230769230771 } })));

  Types.OptionInput Enumeration_input annotation(Placement(transformation(extent={ { -140, -100.0 }, { -100, -60.0 } }), iconTransformation(extent={ { -140, -100.0 }, { -100, -60.0 } })));

  FMI3Float32Output Float32_continuous_output annotation(Placement(transformation(extent={ { 100, 70.0 }, { 120, 90.0 } }), iconTransformation(extent={ { 100, 70.0 }, { 120, 90.0 } })));

  FMI3Float32Output Float32_discrete_output annotation(Placement(transformation(extent={ { 100, 57.69230769230769 }, { 120, 77.6923076923077 } }), iconTransformation(extent={ { 100, 57.69230769230769 }, { 120, 77.6923076923077 } })));

  FMI3Float64Output Float64_continuous_output annotation(Placement(transformation(extent={ { 100, 45.38461538461539 }, { 120, 65.38461538461539 } }), iconTransformation(extent={ { 100, 45.38461538461539 }, { 120, 65.38461538461539 } })));

  FMI3Float64Output Float64_discrete_output annotation(Placement(transformation(extent={ { 100, 33.07692307692307 }, { 120, 53.07692307692307 } }), iconTransformation(extent={ { 100, 33.07692307692307 }, { 120, 53.07692307692307 } })));

  FMI3Int8Output Int8_output annotation(Placement(transformation(extent={ { 100, 20.769230769230766 }, { 120, 40.76923076923077 } }), iconTransformation(extent={ { 100, 20.769230769230766 }, { 120, 40.76923076923077 } })));

  FMI3UInt8Output UInt8_output annotation(Placement(transformation(extent={ { 100, 8.46153846153846 }, { 120, 28.46153846153846 } }), iconTransformation(extent={ { 100, 8.46153846153846 }, { 120, 28.46153846153846 } })));

  FMI3Int16Output Int16_output annotation(Placement(transformation(extent={ { 100, -3.846153846153854 }, { 120, 16.153846153846146 } }), iconTransformation(extent={ { 100, -3.846153846153854 }, { 120, 16.153846153846146 } })));

  FMI3UInt16Output UInt16_output annotation(Placement(transformation(extent={ { 100, -16.15384615384616 }, { 120, 3.8461538461538396 } }), iconTransformation(extent={ { 100, -16.15384615384616 }, { 120, 3.8461538461538396 } })));

  FMI3Int32Output Int32_output annotation(Placement(transformation(extent={ { 100, -28.461538461538467 }, { 120, -8.461538461538467 } }), iconTransformation(extent={ { 100, -28.461538461538467 }, { 120, -8.461538461538467 } })));

  FMI3UInt32Output UInt32_output annotation(Placement(transformation(extent={ { 100, -40.769230769230774 }, { 120, -20.769230769230774 } }), iconTransformation(extent={ { 100, -40.769230769230774 }, { 120, -20.769230769230774 } })));

  FMI3Int64Output Int64_output annotation(Placement(transformation(extent={ { 100, -53.07692307692308 }, { 120, -33.07692307692308 } }), iconTransformation(extent={ { 100, -53.07692307692308 }, { 120, -33.07692307692308 } })));

  FMI3UInt64Output UInt64_output annotation(Placement(transformation(extent={ { 100, -65.38461538461539 }, { 120, -45.38461538461539 } }), iconTransformation(extent={ { 100, -65.38461538461539 }, { 120, -45.38461538461539 } })));

  FMI3BooleanOutput Boolean_output annotation(Placement(transformation(extent={ { 100, -77.69230769230771 }, { 120, -57.69230769230771 } }), iconTransformation(extent={ { 100, -77.69230769230771 }, { 120, -57.69230769230771 } })));

  Types.OptionOutput Enumeration_output annotation(Placement(transformation(extent={ { 100, -90.0 }, { 120, -70.0 } }), iconTransformation(extent={ { 100, -90.0 }, { 120, -70.0 } })));

initial algorithm

  FMI.Internal.loadFMU(
    instance=instance,
    unzipdir=Modelica.Utilities.Files.loadResource("modelica://FMI/Resources/FMUs/9d23b8a"),
    fmiVersion=3,
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

  FMI3SetFloat64(instance, valueReferences={5}, values={Float64_fixed_parameter});
  FMI3SetFloat64(instance, valueReferences={6}, values={Float64_tunable_parameter});
  FMI3SetString(instance, valueReferences={29}, values={String_parameter});

  startTime := time;

  FMI3EnterInitializationMode(instance,
      toleranceDefined=tolerance > 0.0,
      tolerance=tolerance,
      startTime=startTime,
      stopTimeDefined=stopTime < Modelica.Constants.inf,
      stopTime=stopTime);

  FMI3ExitInitializationMode(instance);

algorithm

  when sample(startTime, communicationStepSize) then

    FMI3SetFloat32(instance, valueReferences={1}, values={Float32_continuous_input});
    FMI3SetFloat32(instance, valueReferences={3}, values={Float32_discrete_input});
    FMI3SetFloat64(instance, valueReferences={7}, values={Float64_continuous_input});
    FMI3SetFloat64(instance, valueReferences={9}, values={Float64_discrete_input});
    FMI3SetInt8(instance, valueReferences={11}, values={Int8_input});
    FMI3SetUInt8(instance, valueReferences={13}, values={UInt8_input});
    FMI3SetInt16(instance, valueReferences={15}, values={Int16_input});
    FMI3SetUInt16(instance, valueReferences={17}, values={UInt16_input});
    FMI3SetInt32(instance, valueReferences={19}, values={Int32_input});
    FMI3SetUInt32(instance, valueReferences={21}, values={UInt32_input});
    FMI3SetInt64(instance, valueReferences={23}, values={Int64_input});
    FMI3SetUInt64(instance, valueReferences={25}, values={UInt64_input});
    FMI3SetBoolean(instance, valueReferences={27}, values={Boolean_input});
    FMI3SetInt64(instance, valueReferences={32}, values=Types.OptionToInt64({Enumeration_input}));

    FMI3DoStep(instance,
        currentCommunicationPoint=time,
        communicationStepSize=communicationStepSize);

    Float32_continuous_output := scalar(FMI3GetFloat32(instance, valueReference=2, nValues=1));
    Float32_discrete_output := scalar(FMI3GetFloat32(instance, valueReference=4, nValues=1));
    Float64_continuous_output := scalar(FMI3GetFloat64(instance, valueReference=8, nValues=1));
    Float64_discrete_output := scalar(FMI3GetFloat64(instance, valueReference=10, nValues=1));
    Int8_output := scalar(FMI3GetInt8(instance, valueReference=12, nValues=1));
    UInt8_output := scalar(FMI3GetUInt8(instance, valueReference=14, nValues=1));
    Int16_output := scalar(FMI3GetInt16(instance, valueReference=16, nValues=1));
    UInt16_output := scalar(FMI3GetUInt16(instance, valueReference=18, nValues=1));
    Int32_output := scalar(FMI3GetInt32(instance, valueReference=20, nValues=1));
    UInt32_output := scalar(FMI3GetUInt32(instance, valueReference=22, nValues=1));
    Int64_output := scalar(FMI3GetInt64(instance, valueReference=24, nValues=1));
    UInt64_output := scalar(FMI3GetUInt64(instance, valueReference=26, nValues=1));
    Boolean_output := scalar(FMI3GetBoolean(instance, valueReference=28, nValues=1));
    Enumeration_output := scalar(Types.Int64ToOption(FMI3GetInt64(instance, valueReference=33, nValues=1)));

  end when;

  annotation (
   Icon(coordinateSystem(
      preserveAspectRatio=false,
      extent={{-100,-100},{100,100}}),
      graphics={Bitmap(extent={{-90,-90},{90,90}}, fileName="modelica://FMI/Resources/Images/FMU_bare.svg")}
    ),
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{100,100}})),
    experiment(StopTime=2.0),
    uses(FMI(version="0.0.8")),
    Documentation(info="<html>
<p>For more information open the FMU's <a href=\"modelica://FMI/Resources/FMUs/9d23b8a/documentation/index.html\">original documentation</a>.</p>
</html>")
  );
end Feedthrough;