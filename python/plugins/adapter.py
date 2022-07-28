print("Initializing plugin: ADAPTER.PY")
from time import sleep
from math import sqrt
import time
import typing
import PySide2
import os,sys

# add python plugin dir to sys path so we can import other plugin here and files
sys.path.append(os.getcwd()+"/python/plugins")


from PySide2.QtWidgets import *
from PySide2.QtCore import *
from PySide2.QtGui import *
from ivy.ivy import IvyServer
import pygame.mixer as mixer
import threading as t
from eyeTrack import eyeTrack
from ChatWidget import ChatWidget,chatReader
from search import *
import configAdapter as config


mixer.init()

#class standard pour ivyBus ne pas modifier
class bus(IvyServer):
    def __init__(self, name):
        IvyServer.__init__(self,'adapter')
        self.name = name

#on reinstancie les objets globaux par sécurité + réduire les warnings dans le code en dessous
print("Creating QObject for the UI")
mainWindow=mainWindow
toolbox=toolbox
pprzApp=pprzApp
ivyBus=bus("InterfaceAdapter")

import adapter_setup as a
a.launch(ivyBus,mainWindow,pprzApp)

# Flash écran
# flash=QWidget()

# cercle debug eyetracking
# eye_track=eyeTrack(mainWindow)

# widget instruction
# heading=QLabel("None")
# alt=QLabel("None")
# speed=QLabel("None")
# instruction=QFrame()

# Chat widget
# chat=ChatWidget(mainWindow.centralWidget())

#chat file reader
# chatreader=chatReader(chat,config.chatfile)



# Variables utilisés par les différentes callbacks
print("assigning global variable")
time_fixation=-1
last_point=QPoint(0,0)
looked_at_widget=[]
last_state="HIGH"
radius=5


#callbacks et fonction d'interactions

# def flashAlert(color:QColor,time:float)->None:
#     """affiche un overlay de couleur 'color' pendant 'time' secondes"""
#     flash.setPalette(QPalette(color))
#     flash.show()
#     t.Timer(time/1000,flash.hide).start()


# def flashEyeTrack(point,radius):
#     """fait clignoter le cercle rouge du eyetrack"""
#     eye_track.setGeometry(point.x()-radius/2,point.y()-radius/2,2*radius,2*radius)
#     eye_track.setRadius(radius)
#     eye_track.raise_()
#     eye_track.updateGeometry()
#     eye_track.show()
#     t.Timer(100,eye_track.hide).start()
# def show_back(widget):
#         try:
#             widget.show()
#         except Exception as e:
#             print("Error")
#             print(e)
# def flashWidget(widget):
#     """fait clignoter un widget"""
#     try:
#         widget.hide()
#         t.Timer(WIDGET_FLASH_TIME+0.1,sigBox.showWidget.emit,[widget]).start()
#     except:
#         pass

# def order_3(sender,h,a,s):
#     """affiche un ordre heading,altitude,speed en haut de la carte"""
#     instruction.show()
#     heading.setText(h+"°")
#     alt.setWordWrap(False)
#     alt.setText(a+" m")
#     speed.setText(s+" m/s")
#     # flashAlert(QColor(0,0,255,100),50)
    
# def order_1(sender,msg):
#     """affiche un message en haut de la carte"""
#     instruction.show()
#     heading.setText("")
#     alt.setWordWrap(True)
#     alt.setText(msg)
#     speed.setText("")
    # flashAlert(QColor(0,0,255,100),50)

# def flashColor(red,green,blue,alpha,time):
#     """overload de la fonction flashAlert pour y envoyer des couleur rgb directement"""
#     flashAlert(QColor(red,green,blue,alpha),time)



# def eye_track_update(sender,x,y):
#     """fonction qui récupère la dernière position du eyeTracker et prend la décision de faire
#     flasher un/des widgets ou non"""
#     p=QPoint(float(x),float(y))
#     #check if we are in a fixation, temporary algo!!!! REALLY NEED TO CHANGE!!!!
#     time_fixation=globals()["time_fixation"]
#     last_point=globals()["last_point"]
#     last_state=globals()["last_state"]
#     looked_at_widget=globals()["looked_at_widget"]

#     print(time_fixation)
#     if(sqrt(((p.x()-last_point.x())**2) + ((p.y()-last_point.y())**2))<=100.):
#         if(time_fixation==-1):
#             time_fixation=time.time_ns()
#         radius=10.+((time.time_ns()-time_fixation)/CLOCKS_PER_SEC)*10.
#     else:
#         radius=5
#         time_fixation=-1
#     last_point=p
#     p2=mainWindow.mapFromGlobal(p)
#     t=pprzApp.widgetAt(p)
#     e=t
#     i=0

#     while(e!=None):
#         obj_str=""
#         a=i
#         while(a>0):
#             a-=1
#             obj_str+="  "
#         i+=1
#         print(obj_str+"Widget: "+e.objectName())
#         e=(e.parent())

#     if(t!=None and t!=mainWindow.centralWidget()):
#         if(time.time_ns()-time_fixation>((WIDGET_FLASH_TIME)*CLOCKS_PER_SEC) and time_fixation!=-1):
#             looked_at_widget.append(t)
#         if EYETRACK_DEBUG:flashEyeTrack(p2,radius)
        
    
#     if((time.time_ns()-time_fixation)>((MAX_FIXATION_TIME*CLOCKS_PER_SEC)) and time_fixation!=-1 and last_state=="HIGH"):
#         time_fixation=-1
#         if(len(looked_at_widget)==0):
#             # flashWidget(mainWindow)
#             pass
#         while(len(looked_at_widget)!=0):
#             flashWidget(looked_at_widget.pop())
#     globals()["last_point"]=last_point
#     globals()["time_fixation"]=time_fixation
#     globals()["looked_at_widget"]=looked_at_widget
def debug_ivy(sender,msg):
    print("DEBUG IVY:",msg)

def mentalFatigue(sender,status,index):
    """enregistre le dernier état reçus et fait clignoter en rouge l'écran si status=HIGH"""
    globals()["last_state"]=status
    # if(status=="HIGH"):
    #     flashAlert(QColor(255,0,0,50),50)

# def sendChatMessage(sender,sender_name,msg):
#     """envoie un message dans le chat"""
#     chat.send.emit(sender_name,msg)




# Fonction périodique qui s'éxécute tout les X secondes
# modifiable à souhait
def periodic_verification():
    # if((time.time_ns()-globals()["time_fixation"])>((MAX_FIXATION_TIME*CLOCKS_PER_SEC)) and globals()["time_fixation"]!=-1 and globals()["last_state"]=="HIGH"):
    #     eye_track_update(None,0,0)
    # if(globals()["time_fixation"]==-1):
    #     eye_track_update(None,0,0)
    # chatreader.check_lines()
    pass






#setting up the UI for new graphical component to add.
# def setup_ui(parent):
    #object management

    # pal = QPalette()
    # pal.setColor(QPalette.Window,QColor(255,0,0,100))
    # flash.setAutoFillBackground(True)
    # flash.setPalette(pal)
    # flash.setAttribute(Qt.WA_TransparentForMouseEvents)
    # flash.hide()
    # flash.setObjectName("flashWidget")
    # flash.setParent(parent.centralWidget())
    # flash.raise_()

    # #set size to screen size as resizing would be difficult
    # screen = QGuiApplication.primaryScreen()
    # screenGeometry = screen.geometry()
    # h = screenGeometry.height()
    # w = screenGeometry.width()
    # flash.setGeometry(0,0,2*h,2*w)
    # flash.hide()
    # eye_track.setAttribute(Qt.WA_TransparentForMouseEvents)
    # eye_track.raise_()
    # eye_track.hide()
    # temp=mainWindow.findChild(QHBoxLayout,"MapMainLayout")
    # if(temp!=None):
    #     instruction.setObjectName("Instruction container widget")
    #     instruction.setStyleSheet(".QFrame{background-color: white; border: 1px solid black; border-radius: 3px;}")
    #     hbox=QHBoxLayout()
    #     hbox.setObjectName("Instruction Layout")
    #     instruction.setLayout(hbox)
        
    #     temp.insertWidget(2,instruction,1,Qt.AlignmentFlag.AlignTop|Qt.AlignmentFlag.AlignHCenter)
    #     temp.insertStretch(2,1)

        
    #     heading.setObjectName("heading Label")
    #     alt.setObjectName("alt Label")
    #     speed.setObjectName("speed Label")

    #     hbox.addWidget(heading)
    #     hbox.addSpacing(1)
    #     hbox.addWidget(alt)
    #     hbox.addSpacing(1)
    #     hbox.addWidget(speed)

    #     instruction.setAutoFillBackground(True)
    #     instruction.raise_()
    #     instruction.hide()
        # splitter.addWidget(chat)
        # chat.setStyleSheet(".QFrame{background-color: white; border: 1px solid black; border-radius: 10px;}")


# container object pour les signaux utilisé ci dessous
class SigBox(QObject):
    mentSig=Signal(str,str,str)
    eyeSig=Signal(str,str,str)
    orderSig1=Signal(str,str,str,str)
    orderSig2=Signal(str,str)
    showWidget=Signal(QWidget)
    flashColorsig=Signal(str,str,str,str,str,str)
    periodSig=Signal()
    def __init__(self) -> None:
        super().__init__()
        
# setup_ui(mainWindow)
print("setting up ui")


# On relis des signaux à toutes les callbacks que Ivy peut exploiter -> objectif exécuter
# les callbacks dans le thread principal et pas dans un thread à part
sigBox=SigBox()
sigBox.mentSig.connect(mentalFatigue)
# sigBox.eyeSig.connect(eye_track_update)
# sigBox.orderSig1.connect(order_3)
# sigBox.orderSig2.connect(order_1)
# sigBox.showWidget.connect(show_back)
sigBox.periodSig.connect(periodic_verification)

# les messages Ivy sont reliés aux signaux
ivyBus.bind_msg(lambda a,b,c:sigBox.mentSig.emit(a,b,c),"^MentalFatigue Status=(.*) Index=(.*)")
# ivyBus.bind_msg(lambda a,b,c:sigBox.eyeSig.emit(a,b,c),"^EyeGazePosition X=(.*) Y=(.*)$")
# ivyBus.bind_msg(lambda a,b,c,d:sigBox.orderSig1.emit(a,b,c,d),"Order heading=(.*) alt=(.*) speed=(.*)")
# ivyBus.bind_msg(lambda a,b:sigBox.orderSig2.emit(a,b),"Order info=(.*)")
# ivyBus.bind_msg(lambda s,r,g,b,a,t: sigBox.flashColorsig.emit(s,r,g,b,a,t),"Flash Color=(.*),(.*),(.*),(.*) Time=(.*)")
# ivyBus.bind_msg(sendChatMessage,"Chat sender=(.*) msg=(.*)")
ivyBus.bind_msg(debug_ivy,"(.*)")



# Code lié à l'interruption du bus Ivy
running=True
def kill():
        globals()["running"]=False  
mainWindow.killPlugins.connect(kill)


# code pour la fonction périodique, ne pas toucher
class RepeatTimer(t.Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function(*self.args, **self.kwargs)
Main_Timer=RepeatTimer(0.5,sigBox.periodSig.emit)
Main_Timer.setDaemon(True)
Main_Timer.start()

# Dernière configuration du chat -> choisi le bus sur lequel renvoyé les messages
# chat.setIvyOutput(ivyBus)

# Très Important, Ne pas modifier l'architecture
# Si ajout de nouvelle objet Qt, penser à les détruire ci dessous 
toolbox.plugins().runThreadedScript(["ivyBus.start('"+config.ivy_adress+"')",
                                    "while(globals()['running']):sleep(1)",
                                    "flash.hide()",
                                    "eye_track.hide()",
                                    "instruction.hide()",
                                    "chat.hide()",
                                    "flash.destroy()",
                                    "eye_track.destroy()",
                                    "instruction.destroy()",
                                    "chat.destroy()",
                                    "ivyBus.stop()",
                                    "print('Goodbye!')"
                                    ])


