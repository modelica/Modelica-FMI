within FMI.FMI3.Functions;
impure function FMI3GetInitialFloat32
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
  input Integer nValues;
  output FMI3Float32 values[nValues];

algorithm

  FMI3SetFloat32(instance, float32InputValueReferences, float32InputValues);
  FMI3SetFloat64(instance, float64InputValueReferences, float64InputValues);
  FMI3SetInt32(instance, int32InputValueReferences, int32InputValues);
  FMI3SetBoolean(instance, booleanInputValueReferences, booleanInputValues);

  values := FMI3GetFloat32(instance, valueReference, nValues);

end FMI3GetInitialFloat32;
