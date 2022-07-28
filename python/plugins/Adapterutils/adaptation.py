from ivy.ivy import IvyServer
from configAdapter import *


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

        #checking config for this adaptation
        if name+"_active" in globals().keys():
            if globals()[name+"_active"]==True:
                self.active=globals()[name+"_active"]
        else:
            print("no config found for "+name)
        if name+"_sound" in globals().keys():
            if globals()[name+"_sound"]==True:
                self.sound=True
        
        self.setup=setup_function
        if self.active:
            print(self.name+" is activated, starting setup")
            self.setup(self)
    def add_callback(self,callback):
        self.callbacks.append(callback)
        if self.sound:
            callback.cb_config(self.sound,globals()[self.name+"_sound_file"])



    def run(self,ivy:IvyServer):
        if self.active:
            for c in self.callbacks:
                ivy.bind_msg(lambda *args: c.signal.emit(*args),c.bind_message)    