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
        void flashWidget(QWidget* w);
    private:

        bool debug=false;

        QPoint* last_point;
        clock_t time_fixation;
        QString last_state;
        float last_index;
        QQueue<QWidget*>* looked_at_widget;


        IvyQt* bus;
        qreal radius;
        QWidget* search_pos(QObject* parent,int x, int y,int prof=0);
        QWidget* search_pos(QObject* parent,QPoint* p,int prof=0);
        void flashEyeTrack(int x,int y,int radius);
        void flashEyeTrack(QPoint* p,int radius);
        void setup_ui();

        bool setup;
        QFrame* instruction;
        QLabel* heading;
        QLabel* alt;
        QLabel* speed;
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
