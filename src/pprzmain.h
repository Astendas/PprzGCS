#ifndef PPRZMAIN_H
#define PPRZMAIN_H

#include <QMainWindow>
#include <QtWidgets>
#if defined(ADAPTIVE_ENABLED)
#include "circle_eyetrack.h"
#endif

enum LaunchTypes {
    DEFAULT,
    NORMAL,
    FLIGHTPLAN_EDIT,
    CONFIGURE,
    QUIT,
};

class PprzMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit PprzMain(QWidget *parent = nullptr);
    static LaunchTypes launch_type;
    static int const EXIT_CODE_REBOOT = -123456;
    void setupUi(int width, int height, QWidget* centralWidget);
    #if defined(ADAPTIVE_ENABLED)
    QWidget* getAlertWidget(){return alertWidget;}
    CircleEyeTrack* getEyeTrack(){return eyeTrack;}
    #endif

    void setServerStatus(bool active);
    void setLSLStatus(bool active);


private:
    void newAC(QString ac_id);
    void removeAC(QString ac_id);
    void delete_ac(QString ac_id);
    QWidget* createAlertWidget(QWidget* parent);
    void readWidget(QObject* main,int prof);

    void populate_menu();
    

    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    QMenu* aircraftsTopMenu;
    QMap<QString, QMenu*> acMenus;
    QMap<QString, QAction*> acActions;
    QLabel* serverStatusLed;
    QLabel* LSLStatusLed;

    #if defined(ADAPTIVE_ENABLED)
    QWidget* alertWidget;
    CircleEyeTrack* eyeTrack;
    #endif
};


#endif // PPRZMAIN_H
