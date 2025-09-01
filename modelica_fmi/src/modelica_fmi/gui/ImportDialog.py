from PySide6.QtWidgets import QDialog, QFileDialog
from modelica_fmi.gui.generated.ImportDialog import Ui_ImportDialog


class ImportDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_ImportDialog()
        self.ui.setupUi(self)

        self.ui.selectFMUPathButton.clicked.connect(self.selectFMUPath)
        self.ui.selectModelPathButton.clicked.connect(self.selectModelPath)

    def selectFMUPath(self):
        filename, _ = QFileDialog.getOpenFileName(
            parent=self,
            caption="Select FMU",
            dir=self.ui.fmuPathLineEdit.text(),
            filter="FMUs (*.fmu)",
        )
        if filename:
            self.ui.fmuPathLineEdit.setText(filename)

    def selectModelPath(self):
        filename, _ = QFileDialog.getSaveFileName(
            parent=self,
            caption="Select Modelica File",
            dir=self.ui.modelPathLineEdit.text(),
            filter="Modelica Files (*.mo)",
        )
        if filename:
            self.ui.modelPathLineEdit.setText(filename)
