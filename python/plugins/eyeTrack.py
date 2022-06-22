from PySide2.QtWidgets import QWidget
from PySide2.QtGui import QPainter,QBrush,QPen
from PySide2.QtCore import Qt

import PySide2
import typing

class eyeTrack(QWidget):
    def __init__(self, parent: typing.Optional[PySide2.QtWidgets.QWidget] = ..., f: PySide2.QtCore.Qt.WindowFlags = ...,radius=5) -> None:
        super().__init__(parent, f)
        self.setObjectName("Eye Track Circle")
        self.radius=radius
        self.brush=QBrush(Qt.red)
    def paintEvent(self, event: PySide2.QtGui.QPaintEvent) -> None:
        p=QPainter(self)

        #Draw background
        p.setPen(Qt.red)
        p.setBrush(self.brush)
        p.drawEllipse(0,0,self.radius,self.radius)
        return super().paintEvent(event)
    def setRadius(self,radius):
        self.radius=radius