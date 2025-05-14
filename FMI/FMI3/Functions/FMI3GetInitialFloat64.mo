within FMI.FMI3.Functions;
impure function FMI3GetInitialFloat64
  extends Modelica.Icons.Function;

  import FMI.FMI3.Types.*;
  import FMI.FMI3.Functions.*;

  input Internal.ExternalFMU instance;

  input Real _time;

  input FMI3ValueReference float32InputValueReferences[:] = fill(0, 0);
  input FMI3Float32 float32InputValues[:] = fill(0.0, 0);

  input FMI3ValueReference float64InputValueReferences[:] = fill(0, 0);
  input FMI3Float64 float64InputValues[:] = fill(0.0, 0);

  input FMI3ValueReference int8InputValueReferences[:] = fill(0, 0);
  input FMI3Int8 int8InputValues[:] = fill(0, 0);

  input FMI3ValueReference uint8InputValueReferences[:] = fill(0, 0);
  input FMI3UInt8 uint8InputValues[:] = fill(0, 0);

  input FMI3ValueReference int16InputValueReferences[:] = fill(0, 0);
  input FMI3Int16 int16InputValues[:] = fill(0, 0);

  input FMI3ValueReference uint16InputValueReferences[:] = fill(0, 0);
  input FMI3UInt16 uint16InputValues[:] = fill(0, 0);

  input FMI3ValueReference int32InputValueReferences[:] = fill(0, 0);
  input FMI3Int32 int32InputValues[:] = fill(0, 0);

  input FMI3ValueReference uint32InputValueReferences[:] = fill(0, 0);
  input FMI3UInt32 uint32InputValues[:] = fill(0, 0);

  input FMI3ValueReference int64InputValueReferences[:] = fill(0, 0);
  input FMI3Int64 int64InputValues[:] = fill(0, 0);

  input FMI3ValueReference uint64InputValueReferences[:] = fill(0, 0);
  input FMI3UInt64 uint64InputValues[:] = fill(0, 0);

  input FMI3ValueReference booleanInputValueReferences[:] = fill(0, 0);
  input FMI3Boolean booleanInputValues[:] = fill(false, 0);

  input FMI3ValueReference valueReference;
  input Integer nValues;
  output FMI3Float64 values[nValues];

algorithm

  if size(float32InputValueReferences, 1) > 0 then
    FMI3SetFloat32(instance, float32InputValueReferences, float32InputValues);
  end if;

  if size(float64InputValueReferences, 1) > 0 then
    FMI3SetFloat64(instance, float64InputValueReferences, float64InputValues);
  end if;

  if size(int8InputValueReferences, 1) > 0 then
    FMI3SetInt8(instance, int8InputValueReferences, int8InputValues);
  end if;

  if size(uint8InputValueReferences, 1) > 0 then
    FMI3SetUInt8(instance, uint8InputValueReferences, uint8InputValues);
  end if;

  if size(int16InputValueReferences, 1) > 0 then
    FMI3SetInt16(instance, int16InputValueReferences, int16InputValues);
  end if;

  if size(uint16InputValueReferences, 1) > 0 then
    FMI3SetUInt16(instance, uint16InputValueReferences, uint16InputValues);
  end if;

  if size(int32InputValueReferences, 1) > 0 then
    FMI3SetInt32(instance, int32InputValueReferences, int32InputValues);
  end if;

  if size(uint32InputValueReferences, 1) > 0 then
    FMI3SetUInt32(instance, uint32InputValueReferences, uint32InputValues);
  end if;

  if size(int64InputValueReferences, 1) > 0 then
    FMI3SetInt64(instance, int64InputValueReferences, int64InputValues);
  end if;

  if size(uint64InputValueReferences, 1) > 0 then
    FMI3SetUInt64(instance, uint64InputValueReferences, uint64InputValues);
  end if;

  if size(booleanInputValueReferences, 1) > 0 then
    FMI3SetBoolean(instance, booleanInputValueReferences, booleanInputValues);
  end if;

  values := FMI3GetFloat64(instance, valueReference, nValues);

end FMI3GetInitialFloat64;
