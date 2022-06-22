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

flash=QWidget()



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
    flash.setGeometry(0,0,h,w)
    #flash.hide()
    # eye_track.setAttribute(Qt.WA_TransparentForMouseEvents)
    # eye_track.raise_()
    # eye_track.hide()



setup_ui(mainWindow)
