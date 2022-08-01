print("Initializing plugin: ADAPTER.PY")
try:
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
except:
    print("crash while loading module")
sys.excepthook = sys.__excepthook__
mixer.init()

#class standard pour ivyBus ne pas modifier
class bus(IvyServer):
    def __init__(self, name):
        IvyServer.__init__(self,'adapter')
        self.name = name

#on reinstancie les objets globaux par sécurité + réduire les warnings dans le code en dessous
print("Creating QObject for the UI")
try:
    mainWindow=mainWindow
    toolbox=toolbox
    pprzApp=pprzApp
    ivyBus=bus("InterfaceAdapter")
except:
    print("crash init")
try:
    import adapter_setup as a
    a.launch(ivyBus,mainWindow,pprzApp)
except:
    print("crash while loading adaptation_setup")

def debug_ivy(sender,msg):
    print("DEBUG IVY:",msg)

def mentalFatigue(sender,status,index):
    """enregistre le dernier état reçus et fait clignoter en rouge l'écran si status=HIGH"""
    globals()["last_state"]=status
    # if(status=="HIGH"):
    #     flashAlert(QColor(255,0,0,50),50)




ivyBus.bind_msg(debug_ivy,"(.*)")



# Code lié à l'interruption du bus Ivy
running=True
def kill():
        globals()["running"]=False  
mainWindow.killPlugins.connect(kill)
try:
# Très Important, Ne pas modifier l'architecture
# Si ajout de nouvelle objet Qt, penser à les détruire ci dessous 
    toolbox.plugins().runThreadedScript(["try:",
                                        "  ivyBus.start('"+config.ivy_adress+"')",
                                        "except:",
                                        "   pass",
                                        "while(True):",
                                        "   sleep(1)",
                                        "   if 'running' in globals().keys():\n"
                                        "       if globals()['running']==False:\n",
                                        "           break",
                                        "   else:globals()['running']=True",
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
except:
    pass

