#include "flightplan_viewerv2.h"
#include <QVBoxLayout>
#include <QtWidgets>
#include "AircraftManager.h"
#include <QDebug>
#include <QSettings>
#include "gcs_utils.h"
#include "flightplaneditor.h"

FlightPlanViewerV2::FlightPlanViewerV2(QString ac_id, QWidget *parent) : QTabWidget(parent),
    ac_id(ac_id), current_block(0), current_stage(0), labels_stylesheet("")
{
    setObjectName("FlightPlanViewer");
    addTab(make_blocks_tab(), "Blocks");
    auto flight_plan_editor=new FlightPlanEditor(ac_id, this);
    flight_plan_editor->setObjectName("FlightPlanEditor");
    addTab(flight_plan_editor, "Details");

    connect(AircraftManager::get()->getAircraft(ac_id)->getStatus(),
            &AircraftStatus::nav_status, this, &FlightPlanViewerV2::handleNavStatus);
}


QWidget* FlightPlanViewerV2::make_blocks_tab() {
    auto settings = getAppSettings();
    auto scroll = new QScrollArea(this);
    scroll->setObjectName("FlightPlan Scroll");
    scroll->setWidgetResizable(true);
    scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    auto widget = new QWidget();
    widget->setObjectName("Blocks Tab");

    auto hbox = new QHBoxLayout(widget);
    hbox->setObjectName("Blocks Hbox");
    auto vbox = new QVBoxLayout();
    vbox->setObjectName("Blocks Vbox");
    auto grid_layout = new QGridLayout();
    grid_layout->setObjectName("Blocks Grid Layout");
    grid_layout->setHorizontalSpacing(10);
    grid_layout->setVerticalSpacing(5);
    vbox->addItem(grid_layout);
    vbox->addStretch();
    hbox->addItem(vbox);
    hbox->addStretch();
    hbox->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);

    for(auto block: AircraftManager::get()->getAircraft(ac_id)->getFlightPlan()->getBlocks()) {
        QString icon = block->getIcon();
        QString txt = block->getText();
        QString name = block->getName();

        auto lbl = new QLabel(txt != "" ? txt: name, widget);
        lbl->setObjectName((txt != "" ? txt: name)+" Label");

        auto go_button = new QToolButton(widget);
        go_button->setObjectName("Block Button");
        go_button->setText(QString::fromUtf8("\xE2\x86\x92"));
        connect(go_button, &QPushButton::clicked, this,
            [=]() {
                pprzlink::Message msg(PprzDispatcher::get()->getDict()->getDefinition("JUMP_TO_BLOCK"));
                msg.addField("ac_id", ac_id);
                msg.addField("block_id", block->getNo());
                PprzDispatcher::get()->sendMessage(msg);
            });

        int row = grid_layout->rowCount();
        grid_layout->addWidget(go_button, row, 0);

        if(icon != "") {
            QString icon_path = appConfig()->value("GCS_ICONS_PATH").toString() + "/" + icon;
            auto ll = new QLabel(widget);
            ll->setObjectName((txt != "" ? txt: name)+" Label");
            ll->setPixmap(QPixmap(icon_path));
            ll->setToolTip(txt);
            grid_layout->addWidget(ll, row, 1);
        }

        grid_layout->addWidget(lbl, row, 2);
        grid_layout->setRowStretch(row, 0);

        block_labels.append(lbl);

        connect(go_button, &QToolButton::clicked, [=]() {
            qDebug() << "go to " << name;
        });
    }

    if(block_labels.size() > 0) {
        labels_stylesheet = block_labels[0]->styleSheet();
    }

    widget->setLayout(hbox);
    scroll->setWidget(widget);

    return scroll;
}


void FlightPlanViewerV2::handleNavStatus() {
    auto msg = AircraftManager::get()->getAircraft(ac_id)->getStatus()->getMessage("NAV_STATUS");
    if(msg) {
        uint8_t cur_block, cur_stage;
        //uint32_t block_time, stage_time;
        //float target_lat, target_long, target_climb, target_alt, target_course, dist_to_wp;
        msg->getField("cur_block", cur_block);
        msg->getField("cur_stage", cur_stage);

        QTimer* timer = new QTimer();
        timer->moveToThread(qApp->thread());
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, this, [=]()
        {
            // main thread
            updateNavStatus(cur_block, cur_stage);
            timer->deleteLater();
        });
        QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));

    }
}

void FlightPlanViewerV2::updateNavStatus(uint8_t cur_block, uint8_t cur_stage) {
    //qDebug() << ac_id << "current block: " << cur_block << "  " << cur_stage;

    if(cur_block != current_block || cur_stage != current_stage) {
        //reset idle state
        block_labels[current_block]->setStyleSheet(labels_stylesheet);

        //set "current" state
        block_labels[cur_block]->setStyleSheet("QLabel { background-color: darkGreen; color: white;}");
        current_block = cur_block;
        current_stage = cur_stage;
    }
}
