# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'ImportDialog.ui'
##
## Created by: Qt User Interface Compiler version 6.9.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import QCoreApplication, QMetaObject, QSize, Qt
from PySide6.QtGui import QIcon
from PySide6.QtWidgets import (
    QCheckBox,
    QDialogButtonBox,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QSizePolicy,
    QSpacerItem,
    QToolButton,
    QWidget,
)


class Ui_ImportDialog(object):
    def setupUi(self, ImportDialog):
        if not ImportDialog.objectName():
            ImportDialog.setObjectName("ImportDialog")
        ImportDialog.resize(400, 300)
        icon = QIcon()
        icon.addFile(":/fmi.svg", QSize(), QIcon.Mode.Normal, QIcon.State.Off)
        ImportDialog.setWindowIcon(icon)
        self.gridLayout = QGridLayout(ImportDialog)
        self.gridLayout.setSpacing(12)
        self.gridLayout.setObjectName("gridLayout")
        self.gridLayout.setContentsMargins(12, 12, 12, 12)
        self.mo = QLabel(ImportDialog)
        self.mo.setObjectName("mo")

        self.gridLayout.addWidget(self.mo, 1, 0, 1, 1)

        self.widget = QWidget(ImportDialog)
        self.widget.setObjectName("widget")
        self.horizontalLayout = QHBoxLayout(self.widget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.modelPathLineEdit = QLineEdit(self.widget)
        self.modelPathLineEdit.setObjectName("modelPathLineEdit")

        self.horizontalLayout.addWidget(self.modelPathLineEdit)

        self.selectModelPathButton = QToolButton(self.widget)
        self.selectModelPathButton.setObjectName("selectModelPathButton")

        self.horizontalLayout.addWidget(self.selectModelPathButton)

        self.gridLayout.addWidget(self.widget, 1, 1, 1, 1)

        self.widget_2 = QWidget(ImportDialog)
        self.widget_2.setObjectName("widget_2")
        self.horizontalLayout_2 = QHBoxLayout(self.widget_2)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.fmuPathLineEdit = QLineEdit(self.widget_2)
        self.fmuPathLineEdit.setObjectName("fmuPathLineEdit")

        self.horizontalLayout_2.addWidget(self.fmuPathLineEdit)

        self.selectFMUPathButton = QToolButton(self.widget_2)
        self.selectFMUPathButton.setObjectName("selectFMUPathButton")

        self.horizontalLayout_2.addWidget(self.selectFMUPathButton)

        self.gridLayout.addWidget(self.widget_2, 0, 1, 1, 1)

        self.label = QLabel(ImportDialog)
        self.label.setObjectName("label")

        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)

        self.verticalSpacer = QSpacerItem(
            20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding
        )

        self.gridLayout.addItem(self.verticalSpacer, 5, 0, 1, 1)

        self.buttonBox = QDialogButtonBox(ImportDialog)
        self.buttonBox.setObjectName("buttonBox")
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.StandardButton.Cancel | QDialogButtonBox.StandardButton.Ok
        )

        self.gridLayout.addWidget(self.buttonBox, 6, 0, 1, 2)

        self.basicCheckBox = QCheckBox(ImportDialog)
        self.basicCheckBox.setObjectName("basicCheckBox")

        self.gridLayout.addWidget(self.basicCheckBox, 3, 0, 1, 2)

        self.hideConnectorsCheckBox = QCheckBox(ImportDialog)
        self.hideConnectorsCheckBox.setObjectName("hideConnectorsCheckBox")

        self.gridLayout.addWidget(self.hideConnectorsCheckBox, 4, 0, 1, 2)

        self.retranslateUi(ImportDialog)
        self.buttonBox.accepted.connect(ImportDialog.accept)
        self.buttonBox.rejected.connect(ImportDialog.reject)

        QMetaObject.connectSlotsByName(ImportDialog)

    # setupUi

    def retranslateUi(self, ImportDialog):
        ImportDialog.setWindowTitle(
            QCoreApplication.translate(
                "ImportDialog", "Import an FMU to Modelica", None
            )
        )
        self.mo.setText(
            QCoreApplication.translate("ImportDialog", "Modelica File", None)
        )
        self.selectModelPathButton.setText(
            QCoreApplication.translate("ImportDialog", "...", None)
        )
        self.selectFMUPathButton.setText(
            QCoreApplication.translate("ImportDialog", "...", None)
        )
        self.label.setText(QCoreApplication.translate("ImportDialog", "FMU", None))
        self.basicCheckBox.setText(
            QCoreApplication.translate("ImportDialog", "Basic Co-Simulation", None)
        )
        self.hideConnectorsCheckBox.setText(
            QCoreApplication.translate("ImportDialog", "Hide Connectors", None)
        )

    # retranslateUi
