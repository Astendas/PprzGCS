from datetime import datetime
from random import randint
from PySide2.QtWidgets import QWidget,QScrollArea,QLabel,QVBoxLayout,QHBoxLayout,QFrame,QAbstractScrollArea,QSizePolicy,QLineEdit
from PySide2.QtGui import QPalette,QColor
from PySide2.QtCore import Qt,QEvent,QObject,Signal


class ChatWidget(QFrame):
    send=Signal(str,str)
    def __init__(self,parent=None) -> None:
        super().__init__(parent)
        self.setObjectName("Chat Frame")
        self.last_sender=None
        self.senders={}
        self.nb_message=0
        self.pprzAppThread=None
        self.setup_ui()

        
        

        self.send.connect(self.send_message)
    def setup_ui(self):
        self.scroll_area=QScrollArea(self)
        self.content=QWidget(self.scroll_area)

        self.scroll_area.setObjectName("Chat Scroll Area")
        self.scroll_area.verticalScrollBar().setMaximum(100)
        self.content.setObjectName("Chat Content Widget")

        chat_layout=QVBoxLayout(self)
        self.setMaximumHeight(300)
        self.scroll_area.setWidget(self.content)

        self.setContentsMargins(0,0,0,0)
        self.content.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setSizeAdjustPolicy(QAbstractScrollArea.AdjustToContents)
        self.scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        
        self.layout=QVBoxLayout(self.content)
        self.layout.setObjectName("Chat Content Layout")
        chat_layout.addWidget(self.scroll_area)
        self.layout.addStretch(0)
        self.chatInput=QLineEdit()
        chat_layout.addWidget(self.chatInput)
        self.chatInput.returnPressed.connect(self.self_send)



        #auto resize of content
        self.scroll_area.installEventFilter(self)
    def self_send(self):
        self.send_message("Moi",self.chatInput.text())
        # globals()["ivyBus"].send_msg("Message Sender=Moi msg="+self.chatInput.text())
        self.chatInput.setText("")
    def send_message(self,sender:str,msg:str)->None:
        """
        sender: name of the message sender (will be colorized)
        msg: the message in itself
        """
        messageFrame=QFrame()
        messageFrame.setObjectName("Message Frame")
        messageFrame.setStyleSheet(".QFrame{background-color: rgba(20,20,200,50); border-radius: 2px;}")
        message_layout=QVBoxLayout(messageFrame)
        message_layoutH=QHBoxLayout()
        date=datetime.now()
        senderLabel=QLabel("<u>"+sender+"</u>"+date.strftime("%A %d-%m %H:%M")+" :",messageFrame)
        senderLabel.setTextFormat(Qt.RichText)
        senderLabel.setTextInteractionFlags(Qt.LinksAccessibleByKeyboard|Qt.TextSelectableByKeyboard|Qt.TextSelectableByMouse|Qt.LinksAccessibleByMouse)
        if sender in self.senders.keys():
            color=self.senders[sender]
        else:
            r=randint(1,3)
            if r==3:
                color=QColor(randint(0,16),randint(0,16),randint(150,200),255)
            if r==2:
                color=QColor(randint(0,16),randint(150,200),randint(0,16),255)
            if r==1:
                color=QColor(randint(150,200),randint(0,16),randint(0,16),255)
            self.senders[sender]=color
        pal=QPalette()
        pal.setColor(QPalette.WindowText,color)
        senderLabel.setPalette(pal)

        ChatLabel=QLabel(msg,messageFrame)
        ChatLabel.setTextFormat(Qt.RichText)
        ChatLabel.setOpenExternalLinks(True)
        ChatLabel.setWordWrap(True)
        # ChatLabel.setMinimumWidth(-1)
        # ChatLabel.setMaximumWidth()
        ChatLabel.setTextInteractionFlags(Qt.TextBrowserInteraction|Qt.TextSelectableByKeyboard)
        message_layout.addStretch(1)
        message_layout.addWidget(senderLabel)
        message_layoutH.addSpacing(10)
        message_layoutH.addWidget(ChatLabel)
        message_layout.addLayout(message_layoutH)
        # message_layoutH.addStretch(0)
        # messageFrame.setMaximumWidth(500)


        #add object to message list
        # self.layout.insertWidget(self.nb_message,messageFrame,1,Qt.AlignmentFlag.AlignRight)
        self.layout.insertWidget(self.nb_message,messageFrame)
        self.last_sender=sender
        self.nb_message+=1
        self.scroll_area.verticalScrollBar().setValue(self.scroll_area.verticalScrollBar().maximum())


    def eventFilter(self, watched: QObject, event: QEvent) -> bool:
        if watched==self.content and event==QEvent.Resize:
            self.scroll_area.setMinimumWidth(self.content.minimumSizeHint().width()+self.scroll_area.verticalScrollBar().width())
        max_size=self.width()-80
        for o in self.scroll_area.findChildren(QWidget,"Message Frame"):
            o.setMaximumWidth(max_size)
        return super().eventFilter(watched, event)

    def setThread(self,Thread):
        self.pprzAppThread=Thread