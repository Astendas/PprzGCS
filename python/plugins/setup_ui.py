#import eyeTrack as eye
from time import sleep
import time
import typing
import PySide2
import os
from PySide2.QtWidgets import *
from PySide2.QtCore import *
from PySide2.QtGui import *
from ivy.ivy import IvyServer
import threading as t
from multiprocessing import Process




def setup_ui(parent):
    #object management

    pal = QPalette()
    pal.setColor(QPalette.Window,QColor(255,0,0,100))
    flash.setAutoFillBackground(True)
    flash.setPalette(pal)
    flash.setAttribute(Qt.WA_TransparentForMouseEvents)
    flash.hide()
    flash.setObjectName("flashWidget")
    flash.setParent(parent.centralWidget())
    flash.raise_()

    #set size to screen size as resizing would be difficult
    screen = QGuiApplication.primaryScreen()
    screenGeometry = screen.geometry()
    h = screenGeometry.height()
    w = screenGeometry.width()
    flash.setGeometry(0,0,2*h,2*w)
    flash.hide()
    eye_track.setAttribute(Qt.WA_TransparentForMouseEvents)
    eye_track.raise_()
    eye_track.hide()
    temp=mainWindow.findChild(QHBoxLayout,"MapMainLayout")
    print(temp)
    if(temp!=None):
        instruction.setObjectName("Instruction container widget")
        instruction.setStyleSheet(".QFrame{background-color: white; border: 1px solid black; border-radius: 10px;}")
        hbox=QHBoxLayout()
        hbox.setObjectName("Instruction Layout")
        instruction.setLayout(hbox)
        
        temp.insertWidget(2,instruction,1,Qt.AlignmentFlag.AlignTop|Qt.AlignmentFlag.AlignHCenter)
        temp.insertStretch(2,1)

        
        heading.setObjectName("heading Label")
        alt.setObjectName("alt Label")
        speed.setObjectName("speed Label")

        hbox.addWidget(heading)
        hbox.addSpacing(1)
        hbox.addWidget(alt)
        hbox.addSpacing(1)
        hbox.addWidget(speed)

        instruction.setAutoFillBackground(True)
        instruction.raise_()
        instruction.hide()
