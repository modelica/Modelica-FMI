within FMI.Examples.FMI2;
model ControlledDrive
  Controller_FMU_2 controller_FMU_2(logFMICalls = true)
    annotation (Placement(transformation(extent={{-8,12},{12,32}})));
  Drive_FMU_2 drive_FMU_2
    annotation (Placement(transformation(extent={{40,-42},{60,-22}})));
  Stimuli_FMU_2 stimuli_FMU_2
    annotation (Placement(transformation(extent={{-78,12},{-58,32}})));
equation
  connect(stimuli_FMU_2.w_desired, controller_FMU_2.w_desired) annotation (Line(
        points={{-57,17},{-14,17},{-14,30},{-9,30}}, color={0,0,127}));
  connect(stimuli_FMU_2.LoadTorque_Nm, drive_FMU_2.LoadTorque_Nm) annotation (
      Line(points={{-57,27},{-46,27},{-46,8},{32,8},{32,-40},{39,-40}}, color={
          0,0,127}));
  connect(drive_FMU_2.w, controller_FMU_2.w) annotation (Line(points={{61,-32},
          {66,-32},{66,-46},{-14,-46},{-14,14},{-9,14}}, color={0,0,127}));
  connect(controller_FMU_2.V, drive_FMU_2.V) annotation (Line(points={{13,22},{
          34,22},{34,-24},{39,-24}}, color={0,0,127}));
  annotation (Icon(coordinateSystem(preserveAspectRatio=false)), Diagram(
        coordinateSystem(preserveAspectRatio=false)));
end ControlledDrive;
