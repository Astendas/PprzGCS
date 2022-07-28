from PySide2.QtWidgets import *
from PySide2.QtCore import *
from PySide2.QtGui import *
import pygame.mixer as mixer
import inspect

# container object to create Qt signal for callback use
class callbackBox():
    def __init__(self) -> None:
        super().__init__()
        self.callbackList=[]
        self.sigdic={}
    def add_callback(self,callback,sig):
        self.callbackList.append(callback)
        self.sigdic[callback]=sig

# standard class for ivy callbacks, do not touch
class callback():
    def __init__(self,function,bind_message,callbackBox=None) -> None:
        class sigHolder(QObject):
            sig=Signal([str]*len(inspect.getfullargspec(function).args))
        self.sound=False
        self.sigh=sigHolder()
        self.bind_message=bind_message
        self.func=function
        self.signal=self.sigh.sig
        self.signal.connect(self.run)
        if callbackBox!=None:
            callbackBox.add_callback(self,self.sigh.sig)
    def run(self,*args):
        print(self)
        self.func(*args)
        if self.sound:
            print("Playing sound for callback")
            self.play_sound()


    def cb_config(self,sound_toggle,soundfile=None):
        self.sound=sound_toggle
        self.soundfile=soundfile
    def play_sound(self):
        mixer.music.load(self.soundfile)
        mixer.music.play(1,0.0)