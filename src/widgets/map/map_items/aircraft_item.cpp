#include "aircraft_item.h"
#include "mapwidget.h"
#include "AircraftManager.h"
#include <QApplication>
#include <QDebug>
#include "aircraft.h"
#include "gcs_utils.h"
#include "graphics_icon.h"


AircraftItem::AircraftItem(Point2DLatLon pt, QString ac_id, double neutral_scale_zoom, QObject *parent) :
    MapItem(ac_id, neutral_scale_zoom, parent),
    latlon(pt), heading(0.), last_chunk_index(0)
{
    auto settings = getAppSettings();

    z_value_highlighted = settings.value("map/z_values/aircraft").toDouble();
    z_value_unhighlighted = settings.value("map/z_values/aircraft").toDouble();
    z_value = z_value_unhighlighted;

    auto aircraft = AircraftManager::get()->getAircraft(ac_id);
    int size = settings.value("map/aircraft/size").toInt();

    color_idle = palette.getColor();
    color_unfocused = labelUnfocusedColor(palette.getColor());

    graphics_aircraft = new GraphicsAircraft(palette.getColor(), aircraft->getIcon(), size);
    graphics_text = new GraphicsText(aircraft->name(), palette);
    alarms = new QGraphicsItemGroup();
    bat_alarm = new GraphicsIcon(":/pictures/bat_low.svg", size);
    alarms->addToGroup(bat_alarm);

    for(int i=0; i<settings.value("map/aircraft/track_max_chunk").toInt(); i++) {
        auto gt = new GraphicsTrack(palette);
        graphics_tracks.append(gt);
        QList<Point2DLatLon> l;
        track_chuncks.append(l);
    }

    connect(pprzApp()->toolbox()->watcher(), &Watcher::bat_status, this, &AircraftItem::handle_bat_alarm);

    setZoomFactor(1.1);
}

void AircraftItem::addToMap(MapWidget* map) {

    for(auto gt: graphics_tracks) {
        map->scene()->addItem(gt);
    }

    map->scene()->addItem(graphics_aircraft);
    map->scene()->addItem(graphics_text);
    map->scene()->addItem(alarms);
}

void AircraftItem::updateGraphics(MapWidget* map, uint32_t update_event) {
    if(update_event & (UpdateEvent::ITEM_CHANGED | UpdateEvent::MAP_ZOOMED | UpdateEvent::MAP_ROTATED)) {
        QPointF scene_pos = scenePoint(latlon, zoomLevel(map->zoom()), map->tileSize());
        graphics_aircraft->setPos(scene_pos);
        double s = getScale(map->zoom(), map->scaleFactor());
        graphics_aircraft->setScale(s);
        graphics_aircraft->setRotation(heading);

        double r = map->getRotation();
        auto rot = QTransform().rotate(-r);
        graphics_text->setScale(s);
        graphics_text->setPos(scene_pos + rot.map(QPointF(10, 10)));
        graphics_text->setRotation(-r);

        auto dh = graphics_aircraft->boundingRect().height()/2 + alarms->boundingRect().height();

        alarms->setScale(s);
        alarms->setPos(scene_pos + rot.map(s*QPointF(-alarms->boundingRect().width()/2, -dh)));
        alarms->setRotation(-r);


        for(int i = 0; i<track_chuncks.size(); i++) {
            QPolygonF scenePoints;
            for(auto pt: track_chuncks[i]) {
                scenePoints.append(scenePoint(pt, zoomLevel(map->zoom()), map->tileSize()));
            }
            graphics_tracks[i]->setPoints(scenePoints);
        }
    }
}

void AircraftItem::setPosition(Point2DLatLon pt) {
    latlon = pt;
    auto settings = getAppSettings();
    track_chuncks[last_chunk_index].append(pt);

    if(track_chuncks[last_chunk_index].size() >= settings.value("map/aircraft/track_chunk_size").toInt()) {
        last_chunk_index = (last_chunk_index + 1) % track_chuncks.size();
        assert(track_chuncks[last_chunk_index].size() == 0 || track_chuncks[last_chunk_index].size() == 1);
        if(!track_chuncks[last_chunk_index].isEmpty()) {
            track_chuncks[last_chunk_index].clear();
        }
        track_chuncks[last_chunk_index].append(pt);
    }

    int first_chunk_index = (last_chunk_index + 1) % track_chuncks.size();
    if(!track_chuncks[first_chunk_index].isEmpty()) {
        track_chuncks[first_chunk_index].removeFirst();
    }

    emit itemChanged();
}

void AircraftItem::clearTrack() {
    for(auto tc : track_chuncks) {
        tc.clear();
        last_chunk_index = 0;
    }
    emit itemChanged();
}

void AircraftItem::setHeading(double h) {
    heading = h;
    emit itemChanged();
}

void AircraftItem::setHighlighted(bool h) {
    MapItem::setHighlighted(h);
    graphics_aircraft->setHighlighted(h);
    for(auto gt: graphics_tracks) {
        gt->setHighlighted(h);
    }
    if(h) {
        graphics_text->setDefaultTextColor(color_idle);
    } else {
        graphics_text->setDefaultTextColor(color_unfocused);
    }

    updateZValue();
}

void AircraftItem::updateZValue() {
    auto settings = getAppSettings();
    graphics_aircraft->setZValue(z_value);
    graphics_text->setZValue(z_value);
    alarms->setZValue(z_value);
    double z_tracks = settings.value("map/z_values/aircraft").toDouble();
    for(auto gt: graphics_tracks) {
        gt->setZValue(z_tracks);
    }
}

void AircraftItem::setForbidHighlight(bool fh) {
    graphics_aircraft->setForbidHighlight(fh);
}

void AircraftItem::removeFromScene(MapWidget* map) {
    assert(graphics_aircraft != nullptr);
    map->scene()->removeItem(graphics_aircraft);
    map->scene()->removeItem(graphics_text);
    map->scene()->removeItem(alarms);
    for(auto gt: graphics_tracks) {
        map->scene()->removeItem(gt);
        delete gt;
    }
    graphics_tracks.clear();
    delete graphics_aircraft;
    delete graphics_text;
    delete alarms;
}

void AircraftItem::handle_bat_alarm(QString id, Watcher::BatStatus bs) {
    if(id != ac_id) {
        return;
    }

    switch (bs) {
    case Watcher::BatStatus::CATASTROPHIC:
        bat_alarm->setIcon(":/pictures/bat_catastrophic.svg");
        alarms->show();
        break;
    case Watcher::BatStatus::CRITIC:
        bat_alarm->setIcon(":/pictures/bat_critic.svg");
        alarms->show();
        break;
    case Watcher::BatStatus::LOW:
        bat_alarm->setIcon(":/pictures/bat_low.svg");
        alarms->show();
        break;
    case Watcher::BatStatus::OK:
        alarms->hide();
        break;

    }
}
