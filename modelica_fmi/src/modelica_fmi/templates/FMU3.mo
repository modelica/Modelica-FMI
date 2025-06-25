@@ extends "FMU.mo" @@
@@ block imports @@
  import FMI.FMI3.Types.*;
  import FMI.FMI3.Interfaces.*;
  import FMI.FMI3.Functions.*;
@@ endblock @@
@@ block types @@
@@ for typeDefinition in enumerationTypeDefinitions @@
    function @= typeDefinition.name =@ToInt64

      input @= typeDefinition.name =@ enumerationValues[:];

      output FMI3Int64 integerValues[size(enumerationValues, 1)];

    algorithm

      for i in 1:size(enumerationValues, 1) loop

@@ for item in typeDefinition.items @@
        @@ if loop.index > 1 @@else@@ endif @@if enumerationValues[i] == @= typeDefinition.name =@.@= name(item) =@ then
          integerValues[i] := @= item.value =@;
@@ endfor @@
        else
          assert(false, "Illegal value");
        end if;

      end for;

    end @= typeDefinition.name =@ToInt64;

    function Int64To@= typeDefinition.name =@

      input FMI3Int64 integerValues[:];

      output @= typeDefinition.name =@ enumerationValues[size(integerValues, 1)];

    algorithm

      for i in 1:size(integerValues, 1) loop

@@ for item in typeDefinition.items @@
        @@ if loop.index > 1 @@else@@ endif @@if integerValues[i] == @= item.value =@ then
          enumerationValues[i] := @= typeDefinition.name =@.@= name(item) =@;
@@ endfor @@
        else
          assert(false, "Illegal value");
        end if;

      end for;

    end Int64To@= typeDefinition.name =@;
@@ endfor @@
@@ endblock @@