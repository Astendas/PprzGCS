print("Initializing plugin: ADAPTER.PY")
from time import sleep
from math import sqrt
import time
import typing
import PySide2
import os,sys
try:
    import GCS
except:
    pass
# add python plugin dir to sys path so we can import other plugin here and files
sys.path.append(os.getcwd()+"/python/plugins")


from PySide2.QtWidgets import *
from PySide2.QtCore import *
from PySide2.QtGui import *
from ivy.ivy import IvyServer
import pygame.mixer as mixer
import threading as t
from eyeTrack import eyeTrack
from search import *


# the sound system initialized here can be reused in threaded script!
print("Initializing Sound System")
mixer.init()
sound=mixer.music.load("python/plugins/select.mp3")
WIDGET_FLASH_TIME=2
CLOCKS_PER_SEC=1000000000
MAX_FIXATION_TIME=5

print("Creating QObject for the UI")
mainWindow=mainWindow
toolbox=toolbox
pprzApp=pprzApp
flash=QWidget()
eye_track=eyeTrack(mainWindow)
heading=QLabel("None")
alt=QLabel("None")
speed=QLabel("None")
instruction=QFrame()


print("assigning global variable")
time_fixation=-1
last_point=QPoint(0,0)
looked_at_widget=[]
last_state="HIGH"
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
    t.Timer(100,eye_track.hide).start()

def flashWidget(widget):
    try:
        widget.hide()
        t.Timer(WIDGET_FLASH_TIME,widget.show).start()
    except:
        pass

def order_3(sender,h,a,s):
    instruction.show()
    heading.setText(h+"°")
    alt.setWordWrap(False)
    alt.setText(a+" m")
    speed.setText(s+" m/s")
    flashAlert(QColor(0,0,255,100),50)
    mixer.music.play()
    sleep(3)
    mixer.music.stop()
def order_1(sender,msg):
    instruction.show()
    heading.setText("")
    alt.setWordWrap(True)
    alt.setText(msg)
    speed.setText("")
    flashAlert(QColor(0,0,255,100),50)




def eye_track_update(sender,x,y):
    p=QPoint(float(x),float(y))
    #check if we are in a fixation, temporary algo!!!! REALLY NEED TO CHANGE!!!!
    time_fixation=globals()["time_fixation"]
    last_point=globals()["last_point"]
    last_state=globals()["last_state"]
    looked_at_widget=globals()["looked_at_widget"]

    print(time_fixation)
    if(sqrt(((p.x()-last_point.x())**2) + ((p.y()-last_point.y())**2))<=60.):
        if(time_fixation==-1):
            time_fixation=time.time_ns()
        radius=10.+((time.time_ns()-time_fixation)/CLOCKS_PER_SEC)*10.
    else:
        radius=5
        time_fixation=-1
    last_point=p
    


    # QWidget* w=search_pos(pprzApp()->mainWindow(),p)
    p2=mainWindow.mapFromGlobal(p)
    t=pprzApp.widgetAt(p)
    e=t
    # c=search_for_widgets(mainWindow,p)
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
    # i=0
    # for wid in c:
    #     j=wid
    #     while(j!=None):
    #         obj_str=""
    #         a=i
    #         while(a>0):
    #             a-=1
    #             obj_str+="  "
    #         i+=1
    #         print(obj_str+"Widget: "+j.objectName())
    #         j=(j.parent())

    if(t!=None and t!=mainWindow.centralWidget()):
        if(time.time_ns()-time_fixation>((WIDGET_FLASH_TIME)*CLOCKS_PER_SEC) and time_fixation!=-1):
            looked_at_widget.append(t)
        flashEyeTrack(p2,radius)
        
    
    if((time.time_ns()-time_fixation)>((MAX_FIXATION_TIME*CLOCKS_PER_SEC)) and time_fixation!=-1 and last_state=="HIGH"):
        time_fixation=-1
        if(len(looked_at_widget)==0):
            flashWidget(mainWindow)
        while(len(looked_at_widget)!=0):
            flashWidget(looked_at_widget.pop())
    globals()["last_point"]=last_point
    globals()["time_fixation"]=time_fixation
    globals()["looked_at_widget"]=looked_at_widget
def debug_ivy(sender,msg):
    print("DEBUG IVY:",msg)

def mentalFatigue(sender,status,index):
    globals()["last_state"]=status
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
    temp=mainWindow.findChild(QHBoxLayout,"MapMainLayout")
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




setup_ui(mainWindow)
print("setting up ui")
ivyBus=bus("InterfaceAdapter")
ivyBus.bind_msg(mentalFatigue,"^MentalFatigue Status=(.*) Index=(.*)")
ivyBus.bind_msg(eye_track_update,"EyeGazePosition X=(.*) Y=(.*)")
ivyBus.bind_msg(order_3,"Order heading=(.*) alt=(.*) speed=(.*)")
ivyBus.bind_msg(order_1,"Order info=(.*)")
ivyBus.bind_msg(debug_ivy,"(.*)")
running=True
def kill():
        globals()["running"]=False    
mainWindow.killPlugins.connect(kill)
toolbox.plugins().runThreadedScript(["ivyBus.start('127.255.255.255:2011')",
                                    "while(globals()['running']):sleep(1)",
                                    "flash.hide()",
                                    "eye_track.hide()",
                                    "instruction.hide()",
                                    "flash.destroy()",
                                    "eye_track.destroy()",
                                    "instruction.destroy()",
                                    "ivyBus.stop()",
                                    "print('Goodbye!')"
                                    ])
# ivyBus.start('127.255.255.255:2011')
print("Ivy launched")

