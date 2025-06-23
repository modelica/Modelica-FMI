# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'ImportDialog.ui'
##
## Created by: Qt User Interface Compiler version 6.9.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QAbstractButton, QApplication, QCheckBox, QDialog,
    QDialogButtonBox, QGridLayout, QHBoxLayout, QLabel,
    QLineEdit, QSizePolicy, QSpacerItem, QToolButton,
    QWidget)
from . import icons_rc

class Ui_ImportDialog(object):
    def setupUi(self, ImportDialog):
        if not ImportDialog.objectName():
            ImportDialog.setObjectName(u"ImportDialog")
        ImportDialog.resize(533, 202)
        icon = QIcon()
        icon.addFile(u":/fmi.svg", QSize(), QIcon.Mode.Normal, QIcon.State.Off)
        ImportDialog.setWindowIcon(icon)
        self.gridLayout = QGridLayout(ImportDialog)
        self.gridLayout.setSpacing(12)
        self.gridLayout.setObjectName(u"gridLayout")
        self.gridLayout.setContentsMargins(12, 12, 12, 12)
        self.mo = QLabel(ImportDialog)
        self.mo.setObjectName(u"mo")

        self.gridLayout.addWidget(self.mo, 1, 0, 1, 1)

        self.widget_2 = QWidget(ImportDialog)
        self.widget_2.setObjectName(u"widget_2")
        self.horizontalLayout_2 = QHBoxLayout(self.widget_2)
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.fmuPathLineEdit = QLineEdit(self.widget_2)
        self.fmuPathLineEdit.setObjectName(u"fmuPathLineEdit")

        self.horizontalLayout_2.addWidget(self.fmuPathLineEdit)

        self.selectFMUPathButton = QToolButton(self.widget_2)
        self.selectFMUPathButton.setObjectName(u"selectFMUPathButton")

        self.horizontalLayout_2.addWidget(self.selectFMUPathButton)


        self.gridLayout.addWidget(self.widget_2, 0, 1, 1, 1)

        self.label = QLabel(ImportDialog)
        self.label.setObjectName(u"label")

        self.gridLayout.addWidget(self.label, 0, 0, 1, 1)

        self.verticalSpacer = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)

        self.gridLayout.addItem(self.verticalSpacer, 4, 0, 1, 1)

        self.widget = QWidget(ImportDialog)
        self.widget.setObjectName(u"widget")
        self.horizontalLayout = QHBoxLayout(self.widget)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.modelPathLineEdit = QLineEdit(self.widget)
        self.modelPathLineEdit.setObjectName(u"modelPathLineEdit")

        self.horizontalLayout.addWidget(self.modelPathLineEdit)

        self.selectModelPathButton = QToolButton(self.widget)
        self.selectModelPathButton.setObjectName(u"selectModelPathButton")

        self.horizontalLayout.addWidget(self.selectModelPathButton)


        self.gridLayout.addWidget(self.widget, 1, 1, 1, 1)

        self.hideConnectorsCheckBox = QCheckBox(ImportDialog)
        self.hideConnectorsCheckBox.setObjectName(u"hideConnectorsCheckBox")

        self.gridLayout.addWidget(self.hideConnectorsCheckBox, 2, 0, 1, 2)

        self.buttonBox = QDialogButtonBox(ImportDialog)
        self.buttonBox.setObjectName(u"buttonBox")
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.StandardButton.Cancel|QDialogButtonBox.StandardButton.Ok)

        self.gridLayout.addWidget(self.buttonBox, 6, 0, 1, 2)

        self.hideLargeArraysCheckBox = QCheckBox(ImportDialog)
        self.hideLargeArraysCheckBox.setObjectName(u"hideLargeArraysCheckBox")

        self.gridLayout.addWidget(self.hideLargeArraysCheckBox, 3, 0, 1, 2)


        self.retranslateUi(ImportDialog)
        self.buttonBox.accepted.connect(ImportDialog.accept)
        self.buttonBox.rejected.connect(ImportDialog.reject)

        QMetaObject.connectSlotsByName(ImportDialog)
    # setupUi

    def retranslateUi(self, ImportDialog):
        ImportDialog.setWindowTitle(QCoreApplication.translate("ImportDialog", u"Import an FMU to Modelica", None))
        self.mo.setText(QCoreApplication.translate("ImportDialog", u"Modelica File", None))
        self.selectFMUPathButton.setText(QCoreApplication.translate("ImportDialog", u"...", None))
        self.label.setText(QCoreApplication.translate("ImportDialog", u"FMU", None))
        self.selectModelPathButton.setText(QCoreApplication.translate("ImportDialog", u"...", None))
        self.hideConnectorsCheckBox.setText(QCoreApplication.translate("ImportDialog", u"Hide Connectors", None))
        self.hideLargeArraysCheckBox.setText(QCoreApplication.translate("ImportDialog", u"Hide Large Arrays", None))
    # retranslateUi

