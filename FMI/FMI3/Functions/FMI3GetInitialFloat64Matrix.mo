within FMI.FMI3.Functions;
impure function FMI3GetInitialFloat64Matrix
  extends Modelica.Icons.Function;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Functions.*;

  input Internal.ExternalFMU instance;
  input Real _time;

  input FMI3ValueReference Float64ValueReferences[:];
  input FMI3Float64 Float64Values[:];

  input FMI3ValueReference Int32ValueReferences[:];
  input FMI3Int32 Int32Values[:];

  input FMI3ValueReference BooleanValueReferences[:];
  input FMI3Boolean BooleanValues[:];

  input FMI3ValueReference valueReference;
  input Integer m;
  input Integer n;
  input Integer nValues;
  output Real values[m, n];

algorithm

  FMI3SetFloat64(instance, Float64ValueReferences, Float64Values);
  FMI3SetInt32(instance, Int32ValueReferences, Int32Values);
  FMI3SetBoolean(instance, BooleanValueReferences, BooleanValues);

  values := FMI3.Functions.FMI3GetFloat64Matrix(instance, valueReference, m, n, nValues);

end FMI3GetInitialFloat64Matrix;
