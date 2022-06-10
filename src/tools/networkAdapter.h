#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H
#include <optional>
#include <QtCore>
#include "PprzToolbox.h"
#include "PprzApplication.h"
#include "pprzmain.h"
#include "lsl_cpp.h"
#include <IvyQt/ivyqt.h>
#include <IvyQt/peer.h>


class NetworkAdapter : public PprzTool{
    Q_OBJECT
    public:
        explicit NetworkAdapter(PprzApplication* app, PprzToolbox* toolbox);
        virtual void setToolbox(PprzToolbox* toolbox) override;
        static NetworkAdapter* get() {
        return pprzApp()->toolbox()->networkAdapter();
    }
        void setDebug(bool value){debug=value;};

        
        
        QThread netstart;


    public slots:
        void flashAlertWidget();
        void setLSLStatus();
        void flashAlertWidget(QColor color,int time);
    private:

        bool debug=false;

        IvyQt* bus;
        QPoint* previous_point;
        qreal radius;
        QWidget* search_pos(QObject* parent,int x, int y,int prof=0);
        QWidget* search_pos(QObject* parent,QPoint* p,int prof=0);
        void flashEyeTrack(int x,int y,int radius);
        void flashEyeTrack(QPoint* p,int radius);
};

class netthread : public QThread {
    Q_OBJECT
    signals:
        void signal();
        void inletOpened();
    public:
        void run();
};
#endif // NETWORKADAPTER_H
