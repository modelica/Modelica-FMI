within FMI.FMI3.Functions;
function matrix2vector
  extends Modelica.Icons.Function;

  input Real A[:,:];
  output Real v[size(A, 1) * size(A, 2)];

protected
  Integer m, n, i;

algorithm

  m := size(A, 1);
  n := size(A, 2);

  for i in 1:m loop
    v[1+(i-1)*n:i*n] := A[i, :];
  end for;

end matrix2vector;
