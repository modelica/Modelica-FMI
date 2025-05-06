within FMI.FMI3.Functions;
impure function FMI3GetInitialFloat64Matrix
  extends Modelica.Icons.Function;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Functions.*;

  input Internal.ExternalFMU instance;
  input Real _time;

  input FMI3ValueReference float64InputValueReferences[:];
  input FMI3Float64 float64InputValues[:];

  input FMI3ValueReference int32InputValueReferences[:];
  input FMI3Int32 int32InputValues[:];

  input FMI3ValueReference booleanInputValueReferences[:];
  input FMI3Boolean booleanInputValues[:];

  input FMI3ValueReference valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;
  output FMI3Float64 values[m, n];

algorithm

  FMI3SetFloat64(instance, float64InputValueReferences, float64InputValues);
  FMI3SetInt32(instance, int32InputValueReferences, int32InputValues);
  FMI3SetBoolean(instance, booleanInputValueReferences, booleanInputValues);

  values := FMI3.Functions.FMI3GetFloat64Matrix(instance, valueReference, m, n, nValues);

end FMI3GetInitialFloat64Matrix;
