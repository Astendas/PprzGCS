#main module
from Adapterutils.adaptation import Adaptation
import Adapterutils.callback as cb
from configAdapter import *


#python module
from math import sqrt
import time
import threading as t


#pyside module
from PySide2.QtWidgets import *
from PySide2.QtCore import *
from PySide2.QtGui import *


#custom module for adaptation
from ChatWidget import ChatWidget,chatReader
from eyeTrack import eyeTrack


#can contain every callbacks with useful search function, not essential, don't use if not necessary
cbBox=cb.callbackBox()

class sigBox(QObject): #object that contains signals
    sigEyetrack1=Signal(QWidget)
    def __init__(self) -> None:
        super().__init__()





def launch(ivyBus,mainWindow,pprzApp):







    #Exemple Flash:

    def flashAlert(color:QColor,time:float)->None:
        """affiche un overlay de couleur 'color' pendant 'time' secondes"""
        flash.setPalette(QPalette(color))
        flash.show()
        t.Timer(time/1000,flash.hide).start()
    def flashColor(sender,red,green,blue,alpha,time):
        """overload de la fonction flashAlert pour y envoyer des couleur rgb directement"""
        flashAlert(QColor(int(red),int(green),int(blue),int(alpha)),int(time))
    
    def flash_setup(self):

        global flash # On rend l'objet flash global pour que d'autre adaptation puisse se servir de l'overlay "flash" sans en recréer un.
        flash=QWidget()
        pal = QPalette()
        pal.setColor(QPalette.Window,QColor(255,0,0,100))
        flash.setAutoFillBackground(True)
        flash.setPalette(pal)
        flash.setAttribute(Qt.WA_TransparentForMouseEvents)
        flash.hide()
        flash.setObjectName("flashWidget")
        flash.setParent(mainWindow.centralWidget())
        flash.raise_()

        #set size to screen size as resizing would be difficult
        screen = QGuiApplication.primaryScreen()
        screenGeometry = screen.geometry()
        h = screenGeometry.height()
        w = screenGeometry.width()
        flash.setGeometry(0,0,2*h,2*w)
        flash.hide()

        cb1=cb.callback(flashColor,"Flash Color=(.*),(.*),(.*),(.*) Time=(.*)")
        self.add_callback(cb1)
        self.run(ivyBus) #don't forget this function, it will bind correctly all the callbacks on the correct ivyBus

    redAlert= Adaptation("redAlert",flash_setup)









    #Exemple Chat:

    #one of the callback used here, must be placed before the setup
    def sendChatMessage(sender,sender_name,msg):
        """envoie un message dans le chat"""
        chat.object.send.emit(sender_name,msg)


    #the function is runned when the adaptation is created: create visual object and other
    def chat_setup(self):
        global chatWidget # on rend le widget du Chat global pour que d'autre adaptation vienne le modifier
        chatWidget=ChatWidget(mainWindow.centralWidget())
        self.object=chatWidget
        self.object.setIvyOutput(ivyBus)
        #penser à remplacer la ligne suivante si possible (recherche de children Qt potentiellement bugué)
        splitter=mainWindow.centralWidget().children()[1].children()[0] # Les fonctions de recherches étant potentiellement sources de crash, remplacement avec le chemin direct
        splitter.addWidget(self.object)
        chat_cb=cb.callback(sendChatMessage,"Chat sender=(.*) msg=(.*)")
        self.add_callback(chat_cb)
        self.run(ivyBus) #don't forget this function, it will bind correctly all the callbacks on the correct ivyBus
    chat = Adaptation("chat",chat_setup)








    #Exemple EyeTracking:

    # global value for eyetrack

    global time_fixation
    global last_point
    global looked_at_widget
    global last_state
    global radius
    time_fixation=-1
    last_point=QPoint(0,0)
    looked_at_widget=[]
    last_state="HIGH"
    radius=5


    def flashEyeTrack(point,radius):
        """fait clignoter le cercle rouge du eyetrack"""
        eye_track.setGeometry(point.x()-radius/2,point.y()-radius/2,2*radius,2*radius)
        eye_track.setRadius(radius)
        eye_track.raise_()
        eye_track.updateGeometry()
        eye_track.show()
        t.Timer(100,eye_track.hide).start()
    def show_back(widget):
            try:
                widget.show()
            except Exception as e:
                print("Error")
                print(e)
    def flashWidget(widget):
        """fait clignoter un widget"""
        widget.hide()
        t.Timer(WIDGET_FLASH_TIME+0.1,sigBox.sigEyetrack1.emit,[widget]).start() # on passe ici par un signal pour garder le widget.show dans le thread principal sinon crash potentiel
    def eye_track_update(sender,x,y):
        """fonction qui récupère la dernière position du eyeTracker et prend la décision de faire
        flasher un/des widgets ou non"""
        p=QPoint(float(x),float(y))
        #check if we are in a fixation, temporary algo!!!! REALLY NEED TO CHANGE!!!!
        time_fixation=globals()["time_fixation"]
        last_point=globals()["last_point"]
        last_state=globals()["last_state"]
        looked_at_widget=globals()["looked_at_widget"]

        # eye Fixation checking (more than X seconds with eye velocity < Y)
        if(sqrt(((p.x()-last_point.x())**2) + ((p.y()-last_point.y())**2))<=100.):
            if(time_fixation==-1):
                time_fixation=time.time_ns()
            radius=10.+((time.time_ns()-time_fixation)/CLOCKS_PER_SEC)*10.
        else:
            radius=5
            time_fixation=-1
        last_point=p
        p2=mainWindow.mapFromGlobal(p)
        t=pprzApp.widgetAt(p)
        if(t!=None and t!=mainWindow.centralWidget()):
            if(time.time_ns()-time_fixation>((WIDGET_FLASH_TIME)*CLOCKS_PER_SEC) and time_fixation!=-1):
                looked_at_widget.append(t)
            if EYETRACK_DEBUG:flashEyeTrack(p2,radius)
        if((time.time_ns()-time_fixation)>((MAX_FIXATION_TIME*CLOCKS_PER_SEC)) and time_fixation!=-1 and last_state=="HIGH"):
            time_fixation=-1
            while(len(looked_at_widget)!=0):
                flashWidget(looked_at_widget.pop())
        globals()["last_point"]=last_point
        globals()["time_fixation"]=time_fixation
        globals()["looked_at_widget"]=looked_at_widget
    
    def eyetrack_setup(self):
        global eye_track
        eye_track=eyeTrack(mainWindow)
        self.object=eye_track
        cb1=cb.callback(eye_track_update,"^EyeGazePosition X=(.*) Y=(.*)$")
        self.add_callback(cb1)
        self.run(ivyBus) #don't forget this function, it will bind correctly all the callbacks on the correct ivyBus
    
    eyetracking=Adaptation("eyeTrack",eyetrack_setup)







    # Exemple Instruction:
    def order_3(sender,h,a,s):
        """affiche un ordre heading,altitude,speed en haut de la carte"""
        instruction.show()
        heading.setText(h+"°")
        alt.setWordWrap(False)
        alt.setText(a+" m")
        speed.setText(s+" m/s")
        flashAlert(QColor(0,0,255,100),50)
    def order_1(sender,msg):
        """affiche un message en haut de la carte"""
        instruction.show()
        heading.setText("")
        alt.setWordWrap(True)
        alt.setText(msg)
        speed.setText("")
    def instruction_setup(self):
        global heading,alt,speed,instruction
        heading=QLabel("None")
        alt=QLabel("None")
        speed=QLabel("None")
        instruction=QFrame()
        self.object=instruction
        #penser à remplacer la ligne suivante si possible (recherche de children Qt potentiellement bugué)
        temp=mainWindow.centralWidget().children()[1].children()[1].children()[1].children()[-1].children()# Les fonctions de recherches étant potentiellement sources de crash, remplacement avec le chemin direct
        for o in temp:
            if o.objectName()=="MapMainLayout":
                temp=o
                break
        # temp=mainWindow.findChild(QHBoxLayout,"MapMainLayout")
        instruction.setObjectName("Instruction container widget")
        instruction.setStyleSheet(".QFrame{background-color: white; border: 1px solid black; border-radius: 3px;}")
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

        cb1=cb.callback(order_3,"Order heading=(.*) alt=(.*) speed=(.*)")
        cb2=cb.callback(order_1,"Order info=(.*)")
        self.add_callback(cb1)
        self.add_callback(cb2)

        self.run(ivyBus) #don't forget this function, it will bind correctly all the callbacks on the correct ivyBus
    
    instructionAdaption=Adaptation("instruction",instruction_setup)

