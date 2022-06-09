#include "strip.h"
#include "AircraftManager.h"
#include <QPainter>
#include <QPaintEvent>
#include "dispatcher_ui.h"

Strip::Strip(QString ac_id, QWidget *parent,  bool full) : QWidget(parent), _ac_id(ac_id)
{
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    build_full_strip();
    build_short_strip();
    mainLayout->addWidget(full_strip);
    mainLayout->addWidget(short_strip);

    if(full) {
        short_strip->hide();
    } else {
        full_strip->hide();
    }

    ac_color = AircraftManager::get()->getAircraft(_ac_id)->getColor();
    int hue = ac_color.hue();
    int sat = ac_color.saturation();
    ac_color.setHsv(hue, static_cast<int>(sat*0.2), 255);


    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::engine_status, this, &Strip::updateEngineStatus);
    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::flight_param, this, &Strip::updateFlightParams);
    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::telemetry_status, this, &Strip::updateTelemetryStatus);
    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::fly_by_wire, this, &Strip::updateFBW);
    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::ap_status, this, &Strip::updateApStatus);

    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::nav_status, this, &Strip::updateAltTargetDiff);
    connect(AircraftManager::get()->getAircraft(_ac_id)->getStatus(),
            &AircraftStatus::flight_param, this, &Strip::updateAltTargetDiff);
}

void Strip::setCompact(bool compact) {
    full_strip->setVisible(!compact);
    short_strip->setVisible(compact);
}


void Strip::build_full_strip() {
    full_strip = new QWidget(this);
    auto layout_full = new QHBoxLayout(full_strip);

    auto layout_status = new QVBoxLayout();
    auto lay_bat_link = new QVBoxLayout();
    auto lay_head = new QHBoxLayout();
    auto lay_body = new QHBoxLayout();

    layout_full->addLayout(layout_status);
    layout_status->addLayout(lay_head);
    layout_status->addLayout(lay_body);

    lay_body->addLayout(lay_bat_link);

    full_flight_time_label = new QLabel("00:00:00", full_strip);
    full_flight_time_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lay_head->addWidget(full_flight_time_label);

    full_speed_label = new JaugeLabel(0, 10, "m/s", full_strip);
    full_speed_label->setPrecision(1);
    lay_head->addWidget(full_speed_label);
    full_speed_label->setStatus(true);

    full_throttle_label = new JaugeLabel(0, 100, "%", full_strip);
    full_throttle_label->setPrecision(0);
    lay_head->addWidget(full_throttle_label);


    // TODO get bat min and max
    //AircraftManager::get()->getAircraft(ac_id).getAirframe()
    full_bat_graph = new GraphLabel(8, 14, full_strip);
    full_bat_graph->setUnit("V");
    lay_bat_link->addWidget(full_bat_graph);

    full_link_label = new ColorLabel(0, full_strip);
    full_link_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lay_bat_link->addWidget(full_link_label);

    QVBoxLayout* status_layout = new QVBoxLayout();
    lay_body->addLayout(status_layout);
    status_layout->addWidget(new QLabel("status", full_strip));

    full_ap_mode_label = new ColorLabel(0, full_strip);
    full_ap_mode_label->setToolTip("Navigation mode");
    status_layout->addWidget(full_ap_mode_label);

    full_fbw_mode_label = new ColorLabel(0, full_strip);
    full_fbw_mode_label->setToolTip("Radio Command status");
    status_layout->addWidget(full_fbw_mode_label);


    full_gps_mode_label = new ColorLabel(0, full_strip);
    full_gps_mode_label->setToolTip("GPS status");
    status_layout->addWidget(full_gps_mode_label);

    full_alt_graph = new GraphLabel(0, 120, full_strip);
    full_alt_graph->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    full_alt_graph->setDualText(true);
    full_alt_graph->setSecondayText("+0.0");
    full_alt_graph->setPrecision(0);
    full_alt_graph->setUnit("m");
    full_alt_graph->setIndicator(true);
    lay_body->addWidget(full_alt_graph);
}

void Strip::build_short_strip() {
    short_strip = new QWidget(this);
    short_strip->setObjectName("short stip");
    auto short_strip_layout = new QVBoxLayout(short_strip);
    (void) short_strip_layout;

    // get bat min and max
    short_jl_bat = new JaugeLabel(11., 14., "V", short_strip);
    short_jl_bat->setStatus(true);
    short_strip_layout->addWidget(short_jl_bat);

    short_flight_time_label = new QLabel("00:00:00", short_strip);
    short_flight_time_label->setObjectName("short flight time");
    short_flight_time_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    short_strip_layout->addWidget(short_flight_time_label);

    auto hLay = new QHBoxLayout();
    auto grid = new QGridLayout();
    auto sLay = new QVBoxLayout();

    short_strip_layout->addLayout(hLay);
    hLay->addLayout(grid);
    hLay->addLayout(sLay);

    short_vspeed_indicator = new QLabel(QString::fromUtf8("\xE2\x86\x97"), short_strip);
    grid->addWidget(new QLabel("vs", short_strip), 0, 0);
    grid->addWidget(new QLabel("alt", short_strip), 1, 0);
    grid->addWidget(short_vspeed_indicator, 2, 0);
    grid->addWidget(new QLabel("tg", short_strip), 3, 0);

    short_speed_label = new QLabel("-m/s", short_strip);
    short_alt_label = new QLabel("-m", short_strip);
    short_vspeed_label = new QLabel("+-", short_strip);
    short_target_label = new QLabel("-m", short_strip);

    grid->addWidget(short_speed_label, 0, 1);
    grid->addWidget(short_alt_label, 1, 1);
    grid->addWidget(short_vspeed_label, 2, 1);
    grid->addWidget(short_target_label, 3, 1);

    short_ap_mode_label = new ColorLabel(0, short_strip);
    short_fbw_mode_label = new ColorLabel(0, short_strip);
    short_gps_mode_label = new ColorLabel(0, short_strip);

    sLay->addWidget(short_ap_mode_label);
    sLay->addWidget(short_fbw_mode_label);
    sLay->addWidget(short_gps_mode_label);
}


void Strip::paintEvent(QPaintEvent* e) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRect(rect());
    p.setPen(Qt::NoPen);

    p.fillPath(path, ac_color);
    p.drawPath(path);

    QWidget::paintEvent(e);
}

void Strip::mousePressEvent(QMouseEvent *e) {
    (void)e;
    emit DispatcherUi::get()->ac_selected(_ac_id);
}

void Strip::mouseReleaseEvent(QMouseEvent *e) {
    (void)e;
}

void Strip::updateEngineStatus() {
    auto msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("ENGINE_STATUS");
    if(msg) {
        float bat, throttle;
        msg->getField("throttle", throttle);
        msg->getField("bat", bat);
        full_bat_graph->pushData(bat);
        short_jl_bat->setValue(bat);
        full_throttle_label->setValue(throttle);
    }
}

void Strip::updateApStatus() {
    auto msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("AP_STATUS");
    if(msg) {
        QString ap_mode, lat_mode, horiz_mode, gaz_mode, gps_mode, kill_mode;
        uint32_t flight_time;

        msg->getField("flight_time", flight_time);
        msg->getField("kill_mode", kill_mode);
        msg->getField("ap_mode", ap_mode);
        msg->getField("gps_mode", gps_mode);

        int hours = static_cast<int>(static_cast<int64_t>(flight_time)/3600);
        int minutes = static_cast<int>(static_cast<int64_t>(flight_time)/60 - hours*60);
        int seconds = static_cast<int>(static_cast<int64_t>(flight_time) - minutes*60 -hours*3600);

        QString f_time = QString("%1").arg(hours, 2, 10, QChar('0')) + ":" +
                         QString("%1").arg(minutes, 2, 10, QChar('0')) + ":" +
                         QString("%1").arg(seconds, 2, 10, QChar('0'));
        full_flight_time_label->setText(f_time);
        short_flight_time_label->setText(f_time);

        full_throttle_label->setStatus(kill_mode=="OFF");

        if(ap_mode == "HOME" || ap_mode == "FAILSAFE") {
            full_ap_mode_label->setBrush(Qt::red);
            short_ap_mode_label->setBrush(Qt::red);
        } else if (ap_mode == "MANUAL") {
            full_ap_mode_label->setBrush(QColor(0xffa500));
            short_ap_mode_label->setBrush(QColor(0xffa500));
        } else {
            full_ap_mode_label->setBrush(QColor(0x7ef17e));
            short_ap_mode_label->setBrush(QColor(0x7ef17e));
        }
        full_ap_mode_label->setText(ap_mode);
        short_ap_mode_label->setText(ap_mode);


        if(gps_mode == "NOFIX") {
            full_gps_mode_label->setBrush(Qt::red);
            short_gps_mode_label->setBrush(Qt::red);
        } else if (gps_mode == "NA" || gps_mode == "2D") {
            full_gps_mode_label->setBrush(QColor(0xffa500));
            short_gps_mode_label->setBrush(QColor(0xffa500));
        } else {
            full_gps_mode_label->setBrush(QColor(0x7ef17e));
            short_gps_mode_label->setBrush(QColor(0x7ef17e));
        }
        full_gps_mode_label->setText(gps_mode);
        short_gps_mode_label->setText(gps_mode);

    }
}

void Strip::updateAltTargetDiff() {
    auto nav_status_msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("NAV_STATUS");
    auto flight_param_msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("FLIGHT_PARAM");
    if(nav_status_msg && flight_param_msg) {
        float target_alt;
        nav_status_msg->getField("target_alt", target_alt);

        float alt;
        flight_param_msg->getField("alt", alt);

        auto diff = alt - target_alt;
        short_target_label->setText(QString::number(diff, 'f', 1) + " m");

    }
}

void Strip::updateFlightParams() {
    auto msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("FLIGHT_PARAM");
    if(msg) {
        float speed, alt, climb, agl, airspeed;
        msg->getField("speed", speed);
        msg->getField("alt", alt);
        msg->getField("climb", climb);
        msg->getField("agl", agl);
        msg->getField("airspeed", airspeed);
        full_speed_label->setValue(speed);
        short_speed_label->setText(QString::number(speed, 'f', 1) + " m/s");
        full_alt_graph->pushData(agl);
        short_alt_label->setText(QString::number(agl, 'f', 0) + " m");
        QString txt = QString::number(climb, 'f', 1);
        if(climb > 0) {
            txt = "+" + txt;
        }
        full_alt_graph->setSecondayText(txt);
        short_vspeed_label->setText(txt);
        if(abs(speed) > 0.1) {
            full_alt_graph->setIndicatorAngle((climb/speed)*1.2);
        } else if(abs(climb) > 0.01){
            full_alt_graph->setIndicatorAngle(climb/abs(climb)*0.3);
        } else {
            full_alt_graph->setIndicatorAngle(0);
        }

        if(climb > 0.5) {
            short_vspeed_indicator->setText(QString::fromUtf8("\xE2\x86\x97"));
        } else if(climb < -0.5) {
            short_vspeed_indicator->setText(QString::fromUtf8("\xE2\x86\x98"));
        } else {
            short_vspeed_indicator->setText(QString::fromUtf8("\xE2\x86\x92"));
        }

    }
}


void Strip::updateTelemetryStatus() {
    auto msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("TELEMETRY_STATUS");
    if(msg) {
        float time_since_last_msg;
        msg->getField("time_since_last_msg", time_since_last_msg);
        if(time_since_last_msg > 5) {
            full_link_label->setBrush(Qt::red);
        } else {
            full_link_label->setBrush(QColor(0x7ef17e));
        }

        if(time_since_last_msg > 2) {
            full_link_label->setText(QString::number(time_since_last_msg, 'f', 0));
        } else {
            full_link_label->setText("");
        }
    }
}

void Strip::updateFBW() {
    auto msg = AircraftManager::get()->getAircraft(_ac_id)->getStatus()->getMessage("FLY_BY_WIRE");
    if(msg) {
        QString rc_status, rc_mode;
        msg->getField("rc_status", rc_status);
        msg->getField("rc_mode", rc_mode);

        if(rc_status == "OK") {
            full_fbw_mode_label->setBrush(QColor(0x7ef17e));
            short_fbw_mode_label->setBrush(QColor(0x7ef17e));
        } else if (rc_status == "LOST" || rc_status == "REALLY_LOST") {
            full_fbw_mode_label->setBrush(Qt::red);
            short_fbw_mode_label->setBrush(Qt::red);
        } else {
            full_fbw_mode_label->setBrush(QColor(0xffa500));
            short_fbw_mode_label->setBrush(QColor(0xffa500));
        }

        full_fbw_mode_label->setText(rc_status);
        short_fbw_mode_label->setText(rc_status);

    }
}
