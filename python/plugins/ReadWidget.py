from PySide2.QtWidgets import QWidget
from PySide2.QtCore import Qt,QObject

import PySide2
import typing

def readWidget(main:QWidget,prof:int =0,max_prof=99999):
    if(prof<=max_prof):
        list1 = main.children()
        layer=""
        a=prof
        while(a>0):
            layer+="  "
            a-=1
        print(layer+"Widget :"+main.objectName(),end="")
        try:
            print(" minSize: ",main.minimumSizeHint().height(),",",main.minimumSizeHint().width())
        except Exception as e:
            print("")
        if(len(list1)>0):
            for obj in list1:
                if(main.objectName()!=""):
                    readWidget(obj,prof+1,max_prof)
readWidget(mainWindow,max_prof=5)
# a=mainWindow.findChild(QWidget,"mapimapi")
# b=a.findChild(QWidget,"ButtonsRightLayout")
# b.itemAt(0).hide()
# a.repaint()
    
