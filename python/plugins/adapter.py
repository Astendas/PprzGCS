from time import sleep
from math import sqrt
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

WIDGET_FLASH_TIME=2
CLOCKS_PER_SEC=1000000000
MAX_FIXATION_TIME=5
class eyeTrack(QWidget):
    def __init__(self, parent: typing.Optional[PySide2.QtWidgets.QWidget] = mainWindow,radius=5) -> None:
        super().__init__(parent)
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

print("hello")
flash=QWidget()
eye_track=eyeTrack(mainWindow)

time_fixation=-1
last_point=QPoint(0,0)
looked_at_widget=[]
last_state="LOW"
radius=5


class bus(IvyServer):
    def __init__(self, name,adress='127.255.255.255:2011'):
        IvyServer.__init__(self,'adapter')
        self.name = name
        # self.start(adress)

def flashAlert(color:QColor,time:int)->None:
    
    flash.setPalette(QPalette(color))
    flash.show()
    t.Timer(time/1000,flash.hide).start()

def flashEyeTrack(point,radius):
    eye_track.setGeometry(point.x()-radius/2,point.y()-radius/2,2*radius,2*radius)
    eye_track.setRadius(radius)
    eye_track.raise_()
    eye_track.updateGeometry()
    eye_track.show()
    # t.Timer(10,eye_track.hide).start()

def flashWidget(widget):
    try:
        widget.hide()
        t.Timer(WIDGET_FLASH_TIME,widget.show).start()
    except:
        pass

def eye_track_update(sender,x,y):
    p=QPoint(float(x),float(y))
    #check if we are in a fixation, temporary algo!!!! REALLY NEED TO CHANGE!!!!
    if not "last_point" in locals():
        last_point=QPoint(0,0)
    if not "time_fixation" in locals():
        time_fixation=-1
    if(sqrt(((p.x()-last_point.x())^2) + ((p.y()-last_point.y())^2))<=60.):
        if(time_fixation==-1):
            time_fixation=time.time_ns()
        radius=10.+((time.time_ns()-time_fixation)/CLOCKS_PER_SEC)*100.
    else:
        radius=5
        time_fixation=-1
    last_point=p
    


    # QWidget* w=search_pos(pprzApp()->mainWindow(),p);
    p2=mainWindow.mapFromGlobal(p)
    t=pprzApp.widgetAt(p)
    e=t
    i=0

    while(e!=None):
        obj_str=""
        a=i
        while(a>0):
            a-=1
            obj_str+="  "
        i+=1
        print(obj_str+"Widget: "+e.objectName())
        e=(e.parent())

    if(t!=None and t!=mainWindow.centralWidget()):
        if(time.time_ns()-time_fixation>((WIDGET_FLASH_TIME)*CLOCKS_PER_SEC) and time_fixation!=-1):
            looked_at_widget.append(t)
        #cout<<"widget = "+w->objectName().toStdString()<<endl;
        flashEyeTrack(p2,radius)
        
    
    if((time.time_ns()-time_fixation)>((MAX_FIXATION_TIME*CLOCKS_PER_SEC)) and time_fixation!=-1 and last_state=="HIGH"):
        time_fixation=-1
        if(len(looked_at_widget)==0):
            flashWidget(mainWindow)
        while(len(looked_at_widget)!=0):
            flashWidget(looked_at_widget.pop())
def debug_ivy(sender,msg):
    print("DEBUG IVY:",msg)

def mentalFatigue(sender,status,index):
    last_state=status
    if(status=="HIGH"):
        flashAlert(QColor(0,0,255,20),50)
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



setup_ui(mainWindow)
print("setting up ui")
ivyBus=bus("InterfaceAdapter")
ivyBus.bind_msg(mentalFatigue,"^MentalFatigue Status=(.*) Index=(.*)")
ivyBus.bind_msg(eye_track_update,"EyeGazePosition X=(.*) Y=(.*)")
ivyBus.bind_msg(debug_ivy,"(.*)")
toolbox.plugins().runThreadedScript(["ivyBus.start('127.255.255.255:2011')","while(True):pass"])

