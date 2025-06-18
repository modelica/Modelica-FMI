from PySide6.QtWidgets import (QDialog, QFileDialog, QMessageBox)
from PySide6.QtCore import QSettings
from modelica_fmi.gui.generated.ImportDialog import Ui_ImportDialog
from modelica_fmi.import_fmu_to_modelica import import_fmu_to_modelica


class ImportDialog(QDialog):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_ImportDialog()
        self.ui.setupUi(self)

        self.ui.selectFMUPathButton.clicked.connect(self.selectFMUPath)
        self.ui.selectModelPathButton.clicked.connect(self.selectModelPath)

    def selectFMUPath(self):
        filename, _ = QFileDialog.getOpenFileName(parent=self,
                                                  caption="Select FMU",
                                                  dir=self.ui.fmuPathLineEdit.text(),
                                                  filter="FMUs (*.fmu)")
        if filename:
            self.ui.fmuPathLineEdit.setText(filename)

    def selectModelPath(self):
        filename, _ = QFileDialog.getSaveFileName(parent=self,
                                                  caption="Select Modelica File",
                                                  dir=self.ui.modelPathLineEdit.text(),
                                                  filter="Modelica Files (*.mo)")
        if filename:
            self.ui.modelPathLineEdit.setText(filename)

    def importFMU(self):

        settings = QSettings()
        settings.setValue("fmuPath", self.ui.fmuPathLineEdit.text())
        settings.setValue("modelPath", self.ui.modelPathLineEdit.text())

        try:
            import_fmu_to_modelica(
                fmu_path=self.ui.fmuPathLineEdit.text(),
                model_path=self.ui.modelPathLineEdit.text(),
                basic=self.ui.basicCheckBox.isChecked(),
                hide_connectors=self.ui.hideConnectorsCheckBox.isChecked(),
            )
        except Exception as ex:
            QMessageBox.critical(self, "Failed to import FMU", str(ex))
