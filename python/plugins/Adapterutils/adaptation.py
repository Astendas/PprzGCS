import inspect
from PySide2.QtWidgets import *
from PySide2.QtCore import *
from ivy.ivy import IvyServer
from configAdapter import *
import threading as t

class RepeatTimer(t.Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function(*self.args, **self.kwargs)

# standard class for adaptation, do not touch
class Adaptation():
    def __init__(self,name,setup_function) -> None:
        """
        name: name of the adaptation for the config file, MUST BE UNIQUE
        setup_function: function that will be run when starting the adaptation, contains
                        graphic object creation, callback creation, etc..."""
        self.callbacks=[]
        self.name=name
        self.active=False
        self.sound=False
        self.timers=[]
        self.postIvy=None

        #checking config for this adaptation
        if name+"_active" in globals().keys():
            if globals()[name+"_active"]==True:
                self.active=globals()[name+"_active"]
        else:
            print("no config found for "+name)
        if name+"_sound" in globals().keys():
            self.sound=True
        if name+"_postIvy" in globals().keys():
            self.postIvy=globals()[name+"_postIvy"]
        





        self.setup=setup_function
        if self.active:
            print(self.name+" is activated, starting setup")
            self.setup(self)
    def add_callback(self,callback):
        self.callbacks.append(callback)
        if self.sound:
            callback.sound=self.sound,globals()[self.name+"_sound"]
        if self.postIvy:
            callback.postIvy=self.postIvy
    def add_repeat_callback(self,time,callback,args=None):
        """adds a timed callback (will be executed every "time" seconds)"""
        arglist=inspect.getfullargspec(callback).args
        class repeatTimerSigBox(QObject):
            sig=Signal(*arglist)
        sigBox=repeatTimerSigBox()
        sigBox.sig.connect(callback)
        def test():
            try:
                sigBox.sig.emit()
            except:pass
        Main_Timer=RepeatTimer(time,test,args)
        Main_Timer.setDaemon(True)
        self.timers.append(Main_Timer)




    def run(self,ivy:IvyServer):
        if self.active:
            for c in self.callbacks:
                c.ivyBus=ivy
                ivy.bind_msg(lambda *args: c.signal.emit(*args),c.bind_message)    
            for t in self.timers:
                t.start()