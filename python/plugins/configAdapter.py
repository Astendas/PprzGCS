ivy_adress="127.255.255.255:2011" # Local host
# ivy_adress="192.168.14.255:2011" # for raspberryPI test with vibration motor (victor's phone network IP) TO BE REMOVED

#Listes des widgets (par objectName) qui ne seront pas pris en compte pour le clignotement avec le eye track
widgetBlacklistEyeTrack=["mapimapi","",]

#Listes de interactions
chat_active=True

redAlert_active=True
redAlert_sound=True
redAlert_sound_file="alarm.wav"

eyeTrack_active=True

instruction_active=True


fileChat=True

#fichier qui contient le texte qui sera envoyé automatiquement au chat
chatfile="chat.txt"


#custom config, for global variable you want to add
CLOCKS_PER_SEC=1000000000
WIDGET_FLASH_TIME=0.1
MAX_FIXATION_TIME=5
EYETRACK_DEBUG=False