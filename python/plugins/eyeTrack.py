from PySide2.QtWidgets import QWidget
from PySide2.QtGui import QPainter,QBrush,QPen
from PySide2.QtCore import Qt

import PySide2
import typing

class eyeTrack(QWidget):
    def __init__(self, parent: typing.Optional[PySide2.QtWidgets.QWidget],radius=5) -> None:
        super().__init__(parent)
        self.setObjectName("Eye Track Circle")
        self.radius=radius
        self.brush=QBrush(Qt.red)
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.raise_()
        self.hide()
    def paintEvent(self, event: PySide2.QtGui.QPaintEvent) -> None:
        p=QPainter(self)

        #Draw background
        p.setPen(Qt.red)
        p.setBrush(self.brush)
        p.drawEllipse(0,0,self.radius,self.radius)
        try:
            return super().paintEvent(event)
        except Exception as e:
            pass
        p.end()
    def setRadius(self,radius):
        self.radius=radius