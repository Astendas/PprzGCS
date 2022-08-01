#Bienvenue dans le fichier de configuration 



#option général:
ivy_adress="127.255.255.255:2011" # Local host
# ivy_adress="192.168.14.255:2011" # for raspberryPI test with vibration motor (victor's phone network IP) TO BE REMOVED
# ivy_adress="10.1.127.255:2323" # pour test vibration tablette (sur enac_auth)

#Listes de interactions
# configuration possible:
# X_active=bool         est-ce que c'est activé? (par défaut: False)
# X_sound=file_name     fichier son joué après appelle d'une callback
# X_postIvy=ivyMessage  message ivy envoyé après exécution d'une callback
chat_active=True
redAlert_active=True
# redAlert_postIvy="FlyMate GetAll"
redAlert_sound="alarm.wav"
eyeTrack_active=True
instruction_active=True
chatreader_active=True


#custom config, for global variable you want to add
CLOCKS_PER_SEC=1000000000
WIDGET_FLASH_TIME=0.1
MAX_FIXATION_TIME=5
EYETRACK_DEBUG=False
    #fichier qui contient le texte qui sera envoyé automatiquement au chat
chatfile="chat.txt"
    #Listes des widgets (par objectName) qui ne seront pas pris en compte pour le clignotement avec le eye track
widgetBlacklistEyeTrack=["mapimapi","map"]