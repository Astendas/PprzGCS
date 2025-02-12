#include "listcontainer.h"
#include "dispatcher_ui.h"
#include "AircraftManager.h"
#include "strip.h"

ListContainer::ListContainer(std::function<QWidget*(QString, QWidget*)> constructor, QWidget *parent) :
    ListContainer(constructor, nullptr, parent)
{
    setObjectName("ListContainer");
}

ListContainer::ListContainer(std::function<QWidget*(QString, QWidget*)> constructor,
                             std::function<QWidget*(QString, QWidget*)> alt_constructor,
                             QWidget *parent) : QScrollArea(parent),
    constructor(constructor),
    alt_constructor(alt_constructor)
{
    scroll_content = new QWidget(this);
    scroll_content->setObjectName("Scroll Content");
    scroll_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setWidget(scroll_content);
    setWidgetResizable(true);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto vbox = new QVBoxLayout(scroll_content);
    setWidget(scroll_content);
    vbox->setObjectName("Vertical Layout");
    grid_layout = new QGridLayout();
    grid_layout->setObjectName("Grid Layout");
    vbox->addLayout(grid_layout);
    vbox->addStretch(1);

    scroll_content->installEventFilter(this);

    // // FIXME: small hack to set minimum size of the scrollarea
    // QTimer* t = new QTimer(this);
    // connect(t, &QTimer::timeout, this, [=]() {
    //     this->setMinimumWidth(grid_layout->sizeHint().width());
    // });
    // t->start(500);

    connect(DispatcherUi::get(), &DispatcherUi::new_ac_config, this, &ListContainer::handleNewAC);
    connect(DispatcherUi::get(), &DispatcherUi::ac_deleted, this, &ListContainer::removeAC);
}
bool ListContainer::eventFilter(QObject *o,QEvent *e){
    if(o==scroll_content && e->type()==QEvent::Resize)
    setMinimumWidth(scroll_content->minimumSizeHint().width() + verticalScrollBar()->width());

    return false;
}
void ListContainer::handleNewAC(QString ac_id) { 
    QWidget* pageWidget;
    try {
        pageWidget = constructor(ac_id, this);
        pageWidget->setObjectName("Page Widget container for ac: "+ac_id);
        if(!conf.isNull()) {
            Configurable* c = dynamic_cast<Configurable*>(pageWidget);
            if(c != nullptr) {
                c->configure(conf);
            } else {
                throw std::runtime_error("Widget is not Configurable!!!");
            }
        }
    }  catch (std::runtime_error &e) {
        auto msg = QString(e.what());
        pageWidget = new QWidget(this);
        auto lay = new QVBoxLayout(pageWidget);
        auto label = new QLabel(msg, pageWidget);
        label->setWordWrap(true);
        lay->addWidget(label);
        lay->addStretch();
    }
    widgets[ac_id] = pageWidget;
    auto row = grid_layout->rowCount();
    grid_layout->addWidget(pageWidget, row, 1);


    auto ac = AircraftManager::get()->getAircraft(ac_id);
    auto header = new QWidget(this);
    header->setObjectName("Header Widget");
    auto header_vbox = new QVBoxLayout(header);
    header_vbox->setObjectName("Header Layout");

    auto select_ac_button = new QToolButton(header);
    select_ac_button->setObjectName("Select AC Button");
    select_ac_button->setMinimumSize(QSize(20, 20));
    select_ac_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    select_ac_button->setStyleSheet("background: " + ac->getColor().name());
    header_vbox->addWidget(select_ac_button);
    if(alt_constructor != nullptr) {
        auto action_button = new QToolButton(header);
        action_button->setObjectName("action_button QToolButton");
        action_button->setArrowType(Qt::DownArrow);
        header_vbox->addWidget(action_button);
        QWidget* alt_widget = alt_constructor(ac_id, nullptr);
        alt_widget->setObjectName("Alt Widget");
        alt_widgets[ac_id] = alt_widget;
        alt_widget->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

        connect(action_button, &QToolButton::clicked, this,
            [=]() {
                auto p = action_button->mapToGlobal(action_button->rect().bottomRight());
                alt_widget->move(p);
                alt_widget->show();
            });
    }

    grid_layout->addWidget(header, row, 0);

    buttons[select_ac_button] = ac_id;

    connect(select_ac_button, &QToolButton::clicked, this,
        [=]() {
            emit DispatcherUi::get()->ac_selected(ac_id);
        });

}

void ListContainer::removeAC(QString ac_id) {
    grid_layout->removeWidget(widgets[ac_id]);
    widgets[ac_id]->deleteLater();
    widgets.remove(ac_id);

    auto rect = std::find(buttons.begin(), buttons.end(), ac_id);
    if(rect != buttons.end()) {
        grid_layout->removeWidget(rect.key());
        rect.key()->deleteLater();
        buttons.remove(rect.key());
    }
}
