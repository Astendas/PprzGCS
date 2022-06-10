#include "networkAdapter.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include "iostream"
#include <QtXml>
#include "gcs_utils.h"
#include "pprzmain.h"
#include "lsl_cpp.h"
#include <pthread.h>

using namespace std;

//this function is only used to send test data
/* static void testLSL(){
    //lsl::stream_info info("test", "BioSemi", 1);
	//lsl::stream_outlet outlet(info);
    //cout<<"Outlet started sending data..."<<endl;
	//while (!outlet.wait_for_consumers(120));
    
} */


NetworkAdapter::NetworkAdapter(PprzApplication* app, PprzToolbox* toolbox) : PprzTool(app, toolbox)
{
    setObjectName("NetworkAdapter");
}

void NetworkAdapter::setToolbox(PprzToolbox* toolbox) {
    PprzTool::setToolbox(toolbox);

    //For eyetracking debug
    previous_point=new QPoint(0,0);
    radius=5;


    //create a subthread that receive network data and emit signals for LSL
    QThread *netstart=nullptr;
    netstart = new netthread();
    QObject::connect(netstart,SIGNAL(signal()),this,SLOT(flashAlertWidget()));
    QObject::connect(netstart,SIGNAL(inletOpened()),this,SLOT(setLSLStatus()));
    //netstart->start();


    //Ivy part
    QString ivy_name = "pprznetworkadapter";
    bus=new IvyQt(ivy_name,ivy_name+" ready",this);

    //for debug purpose only, will need to be removed
    bus->bindMessage("(.*)",[=](Peer*,QStringList args){
        if(debug){
            cout<<"Debug log Ivy: "+args.at(0).toStdString()<<endl;
        }
    });


    //bind list
    bus->bindMessage("MentalFatigue Status=(.*) Index=(.*)",[=](Peer*,QStringList args){
        if(args.at(0)=="HIGH"){
            flashAlertWidget();
        }
    });
    bus->bindMessage("FlashRed",[=](Peer*,QStringList args){
        (void)args;
        flashAlertWidget();
    });
    bus->bindMessage("EyeGazePosition X=(.*) Y=(.*)",[=](Peer*,QStringList args){
        QPoint* p=new QPoint(args[0].toInt(),args[1].toInt());
        //check if we are in a fixation, temporary algo
        if(sqrt(((p->x()-previous_point->x())^2) + ((p->y()-previous_point->y())^2))<20){
            radius+=2;
        }else{
            radius=5;
        }

        previous_point=p;
        //QWidget* w=search_pos(pprzApp()->mainWindow(),p);
        QPoint p2=pprzApp()->mainWindow()->mapFromGlobal(*p);
        QWidget* t=pprzApp()->widgetAt(*p);
        QObject* e=t;
        int i=0;
        while(e!=nullptr){
            int a=i;
            while(a--){
                cout<<"  ";
            }
            i++;
            cout<<"Widget: "+e->objectName().toStdString()<<endl;
            e=(e->parent());
        }
        if(t!=nullptr){
            //cout<<"widget = "+w->objectName().toStdString()<<endl;
            if(debug){
                flashEyeTrack(&p2,radius);
            }
        }
    });
    bus->start("127.255.255.255",2011);

    //test eye fixation widget
    //QTimer::singleShot(5000,[&](){flashEyeTrack(400,400,radius);});

    //Thread for sending test
    //std::thread nettest(testLSL);
    //nettest.detach();
}

#if defined(ADAPTIVE_ENABLED)
void NetworkAdapter::flashAlertWidget(QColor color,int time){
    //produce a red flash on the screen
    pprzApp()->mainWindow()->getAlertWidget()->setPalette(QPalette(color));
    pprzApp()->mainWindow()->getAlertWidget()->show();
    QTimer::singleShot(time,[&](){pprzApp()->mainWindow()->getAlertWidget()->hide();});
}
void NetworkAdapter::flashAlertWidget(){
    flashAlertWidget(QColor(255,0,0,13),50);
}

void NetworkAdapter::flashEyeTrack(int x,int y,int radius){
    //create a red circle on the screen
    pprzApp()->mainWindow()->getEyeTrack()->setGeometry(x-radius,y-radius,2*radius,2*radius);
    pprzApp()->mainWindow()->getEyeTrack()->setRadius(radius);
    pprzApp()->mainWindow()->getEyeTrack()->raise();
    pprzApp()->mainWindow()->getEyeTrack()->show();
    QTimer::singleShot(5000,[&](){pprzApp()->mainWindow()->getEyeTrack()->hide();});
}
void NetworkAdapter::flashEyeTrack(QPoint* p,int radius){
    flashEyeTrack(p->x(),p->y(),radius);
}
#endif



QWidget* NetworkAdapter::search_pos(QObject* parent,int x,int y,int prof){
    //return the object at x,y by looking through the widget reccursively
    QObjectList l=parent->children();
    QWidgetList* lw=new QWidgetList();
    QWidget* result=nullptr;
    int nb=0;
    
    //convert QObject to QWidget
    for(QObject* o:l){
        try{
            lw->append(dynamic_cast<QWidget*>(o));
        }
        catch(const std::exception& e){cout<<e.what()<< '\n';}
    }

    //loop that stops when point x,y correspond to nothing or more than
    // 1 child widget (two child widget meaning we look at the parent widget in general)
    for(int i=0;i<lw->size();i++){
        QWidget* w=lw->at(i);
        if(w!=nullptr){
            QRect widgetRect=w->geometry();
            QRect *AbsWidgetRect=new QRect(w->mapToGlobal(widgetRect.topLeft()),w->mapToGlobal(widgetRect.bottomRight()));
            //is the point inside the widget defined boundaries?
            if(AbsWidgetRect->contains(x,y)){
                nb++;
                //does the widget have child?
                if (w->children().size()>0)
                {
                    //we keep track of the depth, still unused
                    result=search_pos(w,x,y,prof+1);
                }
                //more than 1 child widget with contains the point?
                if(nb>1){
                    result=w->parentWidget();
                }
                else{
                    result=w;
                }
            }
        }
    }

    //return search result
    return result;

}
//overload for Qt QPoint
QWidget* NetworkAdapter::search_pos(QObject* parent, QPoint* p,int prof){
    return search_pos(parent,p->x(),p->y(),prof);
}

void NetworkAdapter::setLSLStatus(){
    pprzApp()->mainWindow()->setLSLStatus(true);
}


//ntthread is a QThread in charge of LSL communication, code was removed for now
//netthread running member
void netthread::run(){
    
}
