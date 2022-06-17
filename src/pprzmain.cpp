#include "pprzmain.h"
#include <QMessageBox>
#include <QDebug>
#include "dispatcher_ui.h"
#include "AircraftManager.h"
#include "pprz_dispatcher.h"
#include <QSettings>
#include "app_settings.h"
#include "gcs_utils.h"
#include "circle_eyetrack.h"
#include "python_plugins.h"

#if defined(SPEECH_ENABLED)
#include "speaker.h"
#endif

const int PprzMain::EXIT_CODE_REBOOT = -123456;

LaunchTypes PprzMain::launch_type = DEFAULT;

PprzMain::PprzMain(QWidget *parent) :
    QMainWindow(parent)
{
    setObjectName("PprzMain");
}

void PprzMain::setupUi(int width, int height, QWidget* centralWidget) {

    //creating a dummy container for the central widget
    QWidget* MainContainer=new QWidget();
    MainContainer->setParent(this);
    MainContainer->setObjectName("MainContainer");

    //giving a layout to resize children
    QHBoxLayout* MainLayout = new QHBoxLayout(MainContainer);
    MainLayout->addWidget(centralWidget);

    //add the alert overlay (red flashing overlay)
    #if defined(ADAPTIVE_ENABLED)
    alertWidget=createAlertWidget(MainContainer);
    eyeTrack=new CircleEyeTrack(5,this);
    eyeTrack->setAttribute(Qt::WA_TransparentForMouseEvents);
    eyeTrack->raise();
    eyeTrack->hide();
    #endif
    



    resize(width, height);
    menuBar = new QMenuBar(this);
    menuBar->setObjectName(QString::fromUtf8("menuBar"));
    menuBar->setGeometry(QRect(0, 0, 555, 22));
    setMenuBar(menuBar);
    mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
    addToolBar(Qt::TopToolBarArea, mainToolBar);
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QString::fromUtf8("statusBar"));
    statusBar->layout()->setObjectName("statusBar Layout");
    setStatusBar(statusBar);
    setCentralWidget(MainContainer);
    setWindowIcon(QIcon(":/pictures/icon.svg"));
    populate_menu();
    auto l=new QLabel("server status:");
    l->setObjectName("server status Label");
    statusBar->addPermanentWidget(l);

    serverStatusLed = new QLabel(statusBar);
    serverStatusLed->setObjectName("ServerStatusLed");
    setServerStatus(false);
    statusBar->addPermanentWidget(serverStatusLed);

    #if defined(ADAPTIVE_ENABLED)
    auto l2=new QLabel("LSL status:");
    l2->setObjectName("LSL status Label");
    statusBar->addPermanentWidget(l2);
    LSLStatusLed = new QLabel(statusBar);
    LSLStatusLed->setObjectName("LSLStatusLed");
    setLSLStatus(false);
    statusBar->addPermanentWidget(LSLStatusLed);
    #endif

    connect(DispatcherUi::get(), &DispatcherUi::new_ac_config, this, &PprzMain::newAC);
    connect(DispatcherUi::get(), &DispatcherUi::ac_deleted, this, &PprzMain::removeAC);


}

void PprzMain::setServerStatus(bool active) {
    QIcon ic;
    if(active) {
        ic = QIcon(":/pictures/green_led.svg");
    } else {
        ic = QIcon(":/pictures/red_led.svg");
    }
    serverStatusLed->setPixmap(ic.pixmap(15, 15));
}
void PprzMain::setLSLStatus(bool active) {
    QIcon ic;
    if(active) {
        ic = QIcon(":/pictures/green_led.svg");
    } else {
        ic = QIcon(":/pictures/red_led.svg");
    }
    LSLStatusLed->setPixmap(ic.pixmap(15, 15));
}

void PprzMain::populate_menu() {

    auto file_menu = menuBar->addMenu("&File");
    file_menu->setObjectName("FileMenu");

    auto test = file_menu->addAction("Print Widget in cmd");
    connect(test, &QAction::triggered, [=](){
        readWidget(this,0);
    });

    auto user_dir = file_menu->addAction("Open user directory");
    user_dir->setObjectName("UserDir");
    connect(user_dir, &QAction::triggered, [=](){
        auto settings = getAppSettings();
        QString path = QDir::toNativeSeparators(appConfig()->value("USER_DATA_PATH").toString());
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    auto app_dir = file_menu->addAction("Open app directory");
    app_dir->setObjectName("AppDir");
    connect(app_dir, &QAction::triggered, [=](){
        auto settings = getAppSettings();
        QString path = QDir::toNativeSeparators(appConfig()->value("APP_DATA_PATH").toString());
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    auto edit_settings = file_menu->addAction("Edit Settings");
    edit_settings->setObjectName("EditSettings");
    connect(edit_settings, &QAction::triggered, [=](){
        auto se = new SettingsEditor();
        se->open();
    });

    auto silent_mode_action = file_menu->addAction("Silent mode");
    silent_mode_action->setObjectName("SilentMode");
    silent_mode_action->setCheckable(true);
    silent_mode_action->setChecked(PprzDispatcher::get()->isSilent());

    auto speech_action = file_menu->addAction("Enable Speech");
    speech_action->setObjectName("SpeechAction");
    speech_action->setCheckable(true);
    speech_action->setChecked(speech());

    auto map_track_ac = file_menu->addAction("Track AC");
    map_track_ac->setObjectName("MapTrackAC");
    map_track_ac->setCheckable(true);
    map_track_ac->setChecked(false);

    auto open_flight_plan = file_menu->addAction("Open FlightPlans");
    open_flight_plan->setObjectName("FlightPlan");
    connect(open_flight_plan, &QAction::triggered, [=](){
        auto settings = getAppSettings();
        auto pprz_home = appConfig()->value("PAPARAZZI_HOME").toString();
        auto files = QFileDialog::getOpenFileNames(this, "open fp", pprz_home + "/conf/flight_plans", "*.xml");
        if(PprzMain::launch_type != FLIGHTPLAN_EDIT) {
            PprzMain::launch_type = FLIGHTPLAN_EDIT;
            appConfig()->setValue("FLIGHTPLAN_FILES", files);
            qApp->exit( PprzMain::EXIT_CODE_REBOOT );
        } else {
            for(auto &fp_file: files) {
                qDebug() << "edit flightplan " << fp_file;
                auto name = fp_file.split("/").last();
                AircraftManager::get()->addFPAircraft(name, fp_file);
            }
        }
    });

    connect(silent_mode_action, &QAction::toggled, [=](bool checked) {
        PprzDispatcher::get()->setSilent(checked);
    });

#if defined(SPEECH_ENABLED)
    connect(speech_action, &QAction::toggled, pprzApp()->toolbox()->speaker(), &Speaker::enableSpeech);
#endif

    connect(map_track_ac, &QAction::toggled, [=](bool checked) {
        GlobalConfig::get()->setValue("MAP_TRACK_AC", checked);
    });

    auto quit = file_menu->addAction("&Quit");
    quit->setObjectName("Quit");
    connect(quit, &QAction::triggered, qApp, QApplication::quit);


    aircraftsTopMenu = menuBar->addMenu("&Aircrafts");
    aircraftsTopMenu->setObjectName("AircraftsTopMenu");

    aircraftsTopMenu->addAction("Update", PprzDispatcher::get(), &PprzDispatcher::requestAircrafts);
    auto show_hidden_wp_action = aircraftsTopMenu->addAction("Show hidden waypoints");
    show_hidden_wp_action->setObjectName("ShowHiddenWpAction");
    show_hidden_wp_action->setCheckable(true);

    connect(show_hidden_wp_action, &QAction::toggled, [=](bool show) {
        emit DispatcherUi::get()->showHiddenWaypoints(show);
    });


    

    auto help_menu = menuBar->addMenu("&Help");
    auto about = help_menu->addAction("&About");
    help_menu->setObjectName("Help");
    about->setObjectName("About");

    QString about_txt =  QString(
            "<h1>PprzGCS</h1>"
            "version %1"
            "<p>PprzGCS is a ground control software for <a href=\"https://paparazziuav.org\">paparrazi UAV</a> drones.</p>"
            "<p>source code can be found here: "
            "<a href=\"https://github.com/paparazzi/PprzGCS\">https://github.com/paparazzi/PprzGCS</a></p>"
            "<p>Copyright (C) 2019-2022 Fabien Bonneval (fabien.bonneval@gmail.com)</p>"
            "<p>PprzGCS is free software; you can redistribute it and/or modify "
            "it is under the terms of the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License</a> as published by "
            "the Free Software Foundation; either version 3, or (at your option) "
            "any later version.</p>"
            "<p>The program is provided AS IS with "
            "NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, "
            "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"
            "<h2>Third-party libraries</h2>"
            "<ul>"
            "<li><a href=\"https://www.qt.io/\">Qt 5.12.5</a></li>"
            "<li><a href=\"https://proj.org/\">Proj</a></li>"
            "<li><a %1 href=\"https://libzip.org/\">libzip</a></li>"
            "</ul>").arg(QCoreApplication::applicationVersion());
    connect(about, &QAction::triggered, [=]() {
        QMessageBox::about(this,"About PprzGCS", about_txt);

    });

    auto doc = help_menu->addAction("&Documentation");
    doc->setObjectName("DocumentationAction");

    connect(doc, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://docs.paparazziuav.org/PprzGCS/"));
    });

}


void PprzMain::newAC(QString ac_id) {
    QString acName = AircraftManager::get()->getAircraft(ac_id)->name();
    auto menu = new QMenu("&" + acName);
    auto action = aircraftsTopMenu->addMenu(menu);
    menu->setObjectName(acName+"Menu");
    action->setObjectName(acName+"Action");
    acMenus[ac_id] = menu;
    acActions[ac_id] = action;

    menu->addAction("Remove", this, [ac_id](){
        AircraftManager::get()->removeAircraft(ac_id);
    });
}

void PprzMain::removeAC(QString ac_id) {
    auto action = acActions[ac_id];
    auto menu = acMenus[ac_id];

    acActions.remove(ac_id);
    acMenus.remove(ac_id);

    aircraftsTopMenu->removeAction(action);

    menu->deleteLater();
    action->deleteLater();

}
#if defined(ADAPTIVE_ENABLED)
//@brief add a red overlay on the widget which can be activated by the network tool
QWidget* PprzMain::createAlertWidget(QWidget* parent){
    QWidget* alert= new QWidget();
    QPalette pal = QPalette();
    
    pal.setColor(QPalette::Window,QColor(255,0,0,100));
    alert->setAutoFillBackground(true);
    alert->setPalette(pal);
    alert->setAttribute(Qt::WA_TransparentForMouseEvents);
    alert->hide();
    alert->setObjectName("AlertWidget");
    alert->setParent(parent);
    alert->raise();
    

    //set size to screen size as resizing would be difficult
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int h = screenGeometry.height();
    int w = screenGeometry.width();
    alert->setGeometry(0,0,w,h);
    return alert;
}
void PprzMain::readWidget(QObject* main,int prof){
    QObjectList list = main->children();
    std::string layer="";
    int a=prof;
    while(a--){
        layer+="  ";
    }
    cout<<layer+"Widget :"+main->objectName().toStdString()<<endl;
    if(!list.isEmpty()){
        foreach(auto obj,list){
            if(main->objectName().toStdString()!=""){readWidget(obj,prof+1);}
        }
    }
}
#endif
