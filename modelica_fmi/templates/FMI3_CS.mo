@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;
@@ endblock @@
@@ block parameters @@

  parameter Modelica.Units.SI.Time communicationStepSize = @=communicationStepSize=@ annotation(Dialog(tab="FMI", group="Parameters"));
@@ endblock @@
@@ block equations @@

protected

  parameter Real startTime(fixed=false);
  parameter Boolean startValuesSet(start=false, fixed=false);
  Boolean initialized(start=false, fixed=true);

  record OutputVariables
@@ for variable in outputs @@
    @=fmi_type(variable, True)=@ @=name(variable)=@@=subscripts(variable)=@;
@@ endfor @@
  end OutputVariables;

  OutputVariables outputVariables;

initial algorithm

  if not startValuesSet then
    startTime := time;
@@ for variable in parameters @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, {@=name(variable)=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, @=name(variable)=@);
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, {@=variable.valueReference=@}, @=name(variable)=@, @=numel(variable)=@);
@@ endif @@
@@ endfor @@
    FMI3EnterInitializationMode(instance, tolerance > 0.0, tolerance, startTime, stopTime < Modelica.Constants.inf, stopTime);
@@ for variable in inputs @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, {@=name(variable, '_start')=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, @=name(variable, '_start')=@);
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, {@=variable.valueReference=@}, @=name(variable, '_start')=@, @=numel(variable)=@);
@@ endif @@
@@ endfor @@
    startValuesSet := true;
  end if;

algorithm

  when {initial(), sample(startTime, communicationStepSize)} then

@@ for variable in inputs @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, {pre(@=name(variable)=@)});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(variable)=@(instance, {@=variable.valueReference=@}, pre(@=name(variable)=@));
@@ else @@
    FMI3Set@=fmi_type(variable)=@Matrix(instance, {@=variable.valueReference=@}, pre(@=name(variable)=@), @=numel(variable)=@);
@@ endif @@
@@ endfor @@

    if not initialized and not initial() then
      FMI3ExitInitializationMode(instance);
      initialized := true;
    end if;

    if time >= startTime + communicationStepSize then
      FMI3DoStep(instance, time - communicationStepSize, communicationStepSize);
    end if;

    if not initial() then
@@ for variable in outputs @@
@@ if not variable.dimensions @@
      outputVariables.@=name(variable)=@ := scalar(FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, 1));
@@ elif variable.dimensions|length == 1 @@
      outputVariables.@=name(variable)=@ := FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, @=numel(variable)=@);
@@ else @@
      outputVariables.@=name(variable)=@ := FMI3Get@=fmi_type(variable)=@Matrix(instance, @=variable.valueReference=@, 2, 3, @=numel(variable)=@);
@@ endif @@
@@ endfor @@
    end if;

  end when;

equation
@@ for variable in continuousOutputs @@

  if initial() then
@@ if not variable.dimensions @@
    @=name(variable)=@ = scalar(FMI3GetInitialFloat64(instance, startTime,
        @=dependencies3(variable, 'Float64')=@,
        @=dependencies3(variable, 'Int32')=@,
        @=dependencies3(variable, 'Boolean')=@,
        @=variable.valueReference=@, 1));
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ = FMI3GetInitialFloat64(instance, startTime,
        @=dependencies3(variable, 'Float64')=@,
        @=dependencies3(variable, 'Int32')=@,
        @=dependencies3(variable, 'Boolean')=@,
        @=variable.valueReference=@, size(@=name(variable)=@, 1));
@@ else @@
    @=name(variable)=@ = FMI3GetInitialFloat64Matrix(instance, startTime,
        @=dependencies3(variable, 'Float64')=@,
        @=dependencies3(variable, 'Int32')=@,
        @=dependencies3(variable, 'Boolean')=@,
        @=variable.valueReference=@, size(@=name(variable)=@, 1), size(@=name(variable)=@, 2), product(size(@=name(variable)=@)));
@@ endif @@
  else
    @=name(variable)=@ = outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ for variable in discreteOutputs @@

algorithm

  if initial() then
@@ for dependency in dependencies2(variable) @@
@@ if not variable.dimensions @@
    FMI3Set@=fmi_type(dependency)=@(instance, {@=dependency.valueReference=@}, {@=name(dependency)=@});
@@ elif variable.dimensions|length == 1 @@
    FMI3Set@=fmi_type(dependency)=@(instance, {@=dependency.valueReference=@}, @=name(dependency)=@));
@@ else @@
    FMI3Set@=fmi_type(dependency)=@Matrix(instance, {@=dependency.valueReference=@}, @=name(dependency)=@, product(size(@=name(dependency)=@)));
@@ endif @@
@@ endfor @@
@@ if not variable.dimensions @@
    @=name(variable)=@ := scalar(FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, 1));
@@ elif variable.dimensions|length == 1 @@
    @=name(variable)=@ := FMI3Get@=fmi_type(variable)=@(instance, @=variable.valueReference=@, @=numel(variable)=@);
@@ else @@
    @=name(variable)=@ := FMI3Get@=fmi_type(variable)=@Matrix(instance, @=variable.valueReference=@, size(@=name(variable)=@, 1), size(@=name(variable)=@, 2), product(size(@=name(variable)=@)));
@@ endif @@
  else
    @=name(variable)=@ := outputVariables.@=name(variable)=@;
  end if;
@@ endfor @@
@@ endblock @@