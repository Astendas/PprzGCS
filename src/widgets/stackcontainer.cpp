#include "stackcontainer.h"
#include "dispatcher_ui.h"
#include "AircraftManager.h"
#include <QLabel>

StackContainer::StackContainer(std::function<QWidget*(QString, QWidget*)> constructor,
                               [[maybe_unused]] std::function<QWidget*(QString, QWidget*)> alt_constructor,
                               QWidget *parent) : StackContainer(constructor, parent)
{
    setObjectName("stackContainer");
}

StackContainer::StackContainer(std::function<QWidget*(QString, QWidget*)> constructor, QWidget *parent) : QWidget(parent),
    constructor(constructor)
{
    vLayout = new QVBoxLayout(this);
    vLayout->setObjectName("Vertical Layout");
    vLayout->setSpacing(0);
    contentWidget = new QWidget(this);
    contentWidget->setObjectName("Content Widget");
    stackLayout = new QVBoxLayout(contentWidget);
    stackLayout->setObjectName("StackLayout");
    contentWidget->setAutoFillBackground(true);
    stackLayout->setContentsMargins(0,0,0,0);
    ac_selector = new ACSelector(this);
    ac_selector->setObjectName("AC selector");
    vLayout->addWidget(ac_selector);
    vLayout->addWidget(contentWidget);
    vLayout->setStretch(1, 1);

    connect(DispatcherUi::get(), &DispatcherUi::new_ac_config, this, &StackContainer::handleNewAC);
    connect(DispatcherUi::get(), &DispatcherUi::ac_deleted, this, &StackContainer::removeAC);
    connect(DispatcherUi::get(), &DispatcherUi::ac_selected, this,
            [=](QString ac_id) {
                for(auto &id : viewers_widgets.keys()) {
                    if(id == ac_id) {
                        viewers_widgets[id]->show();
                    } else {
                        viewers_widgets[id]->hide();
                    }
                }
            });

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void StackContainer::handleNewAC(QString ac_id) {
    QWidget* sv;
    try {
        sv = constructor(ac_id,contentWidget);
        sv->setObjectName("container for ac: "+ac_id);
        if(!conf.isNull()) {
            Configurable* c = dynamic_cast<Configurable*>(sv);
            if(c != nullptr) {
                c->configure(conf);
            } else {
                throw std::runtime_error("Widget is not Configurable!!!");
            }
        }
    }  catch (std::runtime_error &e) {
        sv->setObjectName("ERROR");
        auto msg = QString(e.what());
        sv = new QWidget(this);
        auto lay = new QVBoxLayout(sv);
        lay->setObjectName("Layout");
        auto label = new QLabel(msg, sv);
        label->setObjectName("Label");
        label->setWordWrap(true);
        lay->addWidget(label);
        lay->addStretch();
    }
    stackLayout->addWidget(sv);
    viewers_widgets[ac_id] = sv;
    sv->hide();
}

void StackContainer::removeAC(QString ac_id) {
    (void)ac_id;
    auto w = viewers_widgets[ac_id];
    viewers_widgets.remove(ac_id);
    stackLayout->removeWidget(w);
    //viewers_widgets[ac_id]->disconnect();
    w->deleteLater();
}
