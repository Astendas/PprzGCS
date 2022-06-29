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
import threading as t


def search_for_widgets(parent:QWidget,point,depth=0):
    
    global widgets
    widgets=[]
    children=parent.children()

    for child in children:
        try:
            widget=QWidget(child)
            geo=child.geometry()
            if(geo.contains(point)):
                result=search_for_widgets(widget,point,depth+1)
                if len(result)==1:
                    widgets.extend(result)
                else:
                    widgets.append(widget)
        except:
            pass
    return widgets
# def search_for_widgets(parent:QWidget,x,y,depth=0):
#     search_for_widgets(parent,QPoint(x,y),depth=0)