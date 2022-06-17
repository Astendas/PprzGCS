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

//value used in the eyetracking part, used to define the timing
//max fixation time in seconds (can be float) and widget flash time in ms
//MAX FIXATION TIME MUST BE ABOVE THE WIDGET FLASH TIME!!!
//otherwise a widget won't be back before we remove another (can lead to an empty interface)
#define MAX_FIXATION_TIME 1
#define WIDGET_FLASH_TIME 1000



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
    last_point=new QPoint(0,0);
    last_state="LOW";
    last_index=0.;
    looked_at_widget=new QQueue<QWidget*>();
    radius=5;
    setup=false;


    //for instruction widget
    

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
    bus->bindMessage("^(.*)",[=](Peer*,QStringList args){
        if(debug){
            cout<<"Debug log Ivy: "+args.at(0).toStdString()<<endl;
        }
    });


    //bind list
    bus->bindMessage("^MentalFatigue Status=(.*) Index=(.*)",[=](Peer*,QStringList args){
        last_state=args.at(0);
        last_index=args.at(1).toFloat();
        if(args.at(0)=="HIGH"){
            flashAlertWidget();
        }
    });
    bus->bindMessage("^FlashRed",[=](Peer*,QStringList args){
        (void)args;
        flashAlertWidget();
    });
    bus->bindMessage("^EyeGazePosition X=(.*) Y=(.*)",[=](Peer*,QStringList args){
        QPoint* p=new QPoint(args[0].toInt(),args[1].toInt());
        //check if we are in a fixation, temporary algo!!!! REALLY NEED TO CHANGE!!!!
        if(sqrt(((p->x()-last_point->x())^2) + ((p->y()-last_point->y())^2))<=60.){
            if(time_fixation==-1){
                time_fixation=clock();
            }
            radius=10.+((float)(clock()-time_fixation)/(float)CLOCKS_PER_SEC)*100.;
        }
        else{
            radius=5;
            time_fixation=-1;
        }
        last_point=p;
        


        //QWidget* w=search_pos(pprzApp()->mainWindow(),p);
        QPoint p2=pprzApp()->mainWindow()->mapFromGlobal(*p);
        QWidget* t=pprzApp()->widgetAt(*p);
        QObject* e=t;
        int i=0;

        while(e!=nullptr && debug){
            int a=i;
            while(a--){
                cout<<"  ";
            }
            i++;
            cout<<"Widget: "+e->objectName().toStdString()<<endl;
            e=(e->parent());
        }

        if(t!=nullptr and t!=pprzApp()->mainWindow()->centralWidget()){
            if(clock()-time_fixation>(clock_t)((WIDGET_FLASH_TIME/1000)*CLOCKS_PER_SEC) && time_fixation!=-1){
                looked_at_widget->enqueue(t);
            }
            //cout<<"widget = "+w->objectName().toStdString()<<endl;
            if(debug){
                flashEyeTrack(&p2,radius);
            }
        }
        if((clock()-time_fixation)>((clock_t)(MAX_FIXATION_TIME*CLOCKS_PER_SEC)) && time_fixation!=-1 && last_state=="HIGH"){
            time_fixation=-1;
            if(looked_at_widget->isEmpty()){
                flashWidget(pprzApp()->mainWindow());
            }
            while(!looked_at_widget->isEmpty()){
                flashWidget(looked_at_widget->dequeue());
            }
        }
    });
    bus->bindMessage("^ChangeLayout Top=(.*)",[&](Peer*,QStringList args){
        QString widget_name=args.at(0);
        QWidget* home=pprzApp()->mainWindow();
        QWidget* w=(home->findChild<QWidget*>(widget_name));
        cout<<"changing layout"<<endl;
        QWidget* parent=w->parentWidget();
        parent->layout()->removeWidget(w);
        parent->layout()->addWidget(w);

        if(w!=nullptr){
            w->raise();
            cout<<w->objectName().toStdString()<<endl;
        }else{
            cout<<"widget nullptr"<<endl;
        }
    });
    bus->bindMessage("Order heading=(.*) alt=(.*) speed=(.*)",[=](Peer*,QStringList args){
        if(!setup){setup_ui();}
        auto h=args[0].toFloat();
        auto a=args[1].toFloat();
        auto s=args[2].toFloat();
        heading->setText(args[0]+"°");
        alt->setWordWrap(false);
        alt->setText(args[1]+" m");
        speed->setText(args[2]+ " m/s");

        instruction->show();
        flashAlertWidget(QColor(0,0,255,40),100);
    });
    bus->bindMessage("Order info=(.*)",[=](Peer*,QStringList args){
        if(!setup){setup_ui();}
        auto text=args[0];
        heading->setText("");
        alt->setWordWrap(true);
        alt->setText(text);
        speed->setText("");
    });
    bus->bindMessage("Hide w=(.*)",[&](Peer*,QStringList args){
        QWidget* w=pprzApp()->mainWindow()->centralWidget()->findChild<QWidget*>(QString(args.at(0)));
        if(w!=nullptr){
            w->hide();
        }
    });
    bus->bindMessage("setup",[=](Peer*,QStringList args){
        (void)args;
        if(!setup){setup_ui();}
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
    pprzApp()->mainWindow()->getEyeTrack()->setGeometry(x-radius/2,y-radius/2,2*radius,2*radius);
    pprzApp()->mainWindow()->getEyeTrack()->setRadius(radius);
    pprzApp()->mainWindow()->getEyeTrack()->raise();
    pprzApp()->mainWindow()->getEyeTrack()->show();
    QTimer::singleShot(5000,[&](){pprzApp()->mainWindow()->getEyeTrack()->hide();});
}
void NetworkAdapter::flashEyeTrack(QPoint* p,int radius){
    flashEyeTrack(p->x(),p->y(),radius);
}
#endif
void NetworkAdapter::flashWidget(QWidget* w){
    w->setVisible(false);
    QTimer::singleShot(WIDGET_FLASH_TIME,[=](){w->setVisible(true);});
}


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
QWidget* search(QWidget* home,const QString widget_name){
    
    /*  QWidget* w;
        if(w==nullptr){
        QQueue<QWidget*>* q=new QQueue<QWidget*>();
        for(QObject* w:home->children()){
            try{
                q->enqueue(dynamic_cast<QWidget*>(w));
            }catch(const std::exception& e){}
        }
        while(w==nullptr){
            w=search(q->dequeue(),widget_name);
        }
    } 
    return w;*/
}
//overload for Qt QPoint
QWidget* NetworkAdapter::search_pos(QObject* parent, QPoint* p,int prof){
    return search_pos(parent,p->x(),p->y(),prof);
}
void NetworkAdapter::setup_ui(){
    auto temp=pprzApp()->mainWindow()->centralWidget()->findChild<QWidget*>("map2d")->findChild<QHBoxLayout*>("MapMainLayout");

    instruction=new QFrame();
    instruction->setObjectName("Instruction container widget");
    instruction->setStyleSheet(".QFrame{background-color: white; border: 1px solid black; border-radius: 10px;}");
    QHBoxLayout* hbox=new QHBoxLayout();
    hbox->setObjectName("Instruction Layout");
    instruction->setLayout(hbox);
    
    temp->insertWidget(2,instruction,1,Qt::AlignmentFlag::AlignTop|Qt::AlignmentFlag::AlignHCenter);
    temp->insertStretch(2,1);

    heading=new QLabel("None");
    alt=new QLabel("None");
    speed=new QLabel("None");
    heading->setObjectName("heading Label");
    alt->setObjectName("alt Label");
    speed->setObjectName("speed Label");

    hbox->addWidget(heading);
    hbox->addSpacing(1);
    hbox->addWidget(alt);
    hbox->addSpacing(1);
    hbox->addWidget(speed);

    instruction->setAutoFillBackground(true);
    instruction->raise();
    instruction->hide();

    setup=true;
}

void NetworkAdapter::setLSLStatus(){
    pprzApp()->mainWindow()->setLSLStatus(true);
}


//ntthread is a QThread in charge of LSL communication, code was removed for now
//netthread running member
void netthread::run(){
    
}
