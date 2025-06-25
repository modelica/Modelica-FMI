@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI2.Types.*;
  import FMI.FMI2.Interfaces.*;
  import FMI.FMI2.Functions.*;
@@ endblock @@
@@ block types @@
@@ for typeDefinition in enumerationTypeDefinitions @@
    function @= typeDefinition.name =@ToInteger

      input @= typeDefinition.name =@ enumerationValue;

      output FMI2Integer integerValue;

    algorithm

@@ for item in typeDefinition.items @@
      @@ if loop.index > 1 @@else@@ endif @@if enumerationValue == @= typeDefinition.name =@.@= name(item) =@ then
        integerValue := @= item.value =@;
@@ endfor @@
      else
        assert(false, "Illegal value");
      end if;

    end @= typeDefinition.name =@ToInteger;

    function IntegerTo@= typeDefinition.name =@

      input FMI2Integer integerValue;

      output @= typeDefinition.name =@ enumerationValue;

    algorithm

@@ for item in typeDefinition.items @@
      @@ if loop.index > 1 @@else@@ endif @@if integerValue == @= item.value =@ then
        enumerationValue := @= typeDefinition.name =@.@= name(item) =@;
@@ endfor @@
      else
        assert(false, "Illegal value");
      end if;

    end IntegerTo@= typeDefinition.name =@;
@@ endfor @@
@@ endblock @@