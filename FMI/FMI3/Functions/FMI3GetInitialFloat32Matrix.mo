within FMI.FMI3.Functions;
impure function FMI3GetInitialFloat32Matrix
  extends Modelica.Icons.Function;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Functions.*;

  input Internal.ExternalFMU instance;
  input Real _time;

  input FMI3ValueReference float32InputValueReferences[:] = fill(0, 0);
  input FMI3Float32 float32InputValues[:] = fill(0.0, 0);

  input FMI3ValueReference float64InputValueReferences[:] = fill(0, 0);
  input FMI3Float64 float64InputValues[:] = fill(0.0, 0);

  input FMI3ValueReference int32InputValueReferences[:] = fill(0, 0);
  input FMI3Int32 int32InputValues[:] = fill(0, 0);

  input FMI3ValueReference booleanInputValueReferences[:] = fill(0, 0);
  input FMI3Boolean booleanInputValues[:] = fill(false, 0);

  input FMI3ValueReference valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;
  output FMI3Float32 values[m, n];

algorithm

  if size(float32InputValueReferences, 1) > 0 then
    FMI3SetFloat32(instance, float32InputValueReferences, float32InputValues);
  end if;

  if size(float64InputValueReferences, 1) > 0 then
    FMI3SetFloat64(instance, float64InputValueReferences, float64InputValues);
  end if;

  if size(int32InputValueReferences, 1) > 0 then
    FMI3SetInt32(instance, int32InputValueReferences, int32InputValues);
  end if;

  if size(booleanInputValueReferences, 1) > 0 then
    FMI3SetBoolean(instance, booleanInputValueReferences, booleanInputValues);
  end if;

  values := FMI3.Functions.FMI3GetFloat32Matrix(instance, valueReference, m, n, nValues);

end FMI3GetInitialFloat32Matrix;
