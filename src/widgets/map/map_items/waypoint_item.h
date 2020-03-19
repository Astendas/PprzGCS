#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include "map_item.h"
#include <QBrush>
#include "graphics_point.h"
#include "waypoint.h"
#include <memory>

class WaypointItem : public MapItem
{
        Q_OBJECT
public:
    WaypointItem(Point2DLatLon pt, QString ac_id, qreal z_value, MapWidget* map, double neutral_scale_zoom = 15, QObject *parent = nullptr);
    WaypointItem(shared_ptr<Waypoint> wp, QString ac_id, qreal z_value, MapWidget* map, double neutral_scale_zoom = 15, QObject *parent = nullptr);

    Point2DLatLon position() {return Point2DLatLon(waypoint);}
    shared_ptr<Waypoint> getWaypoint() {return waypoint;}
    void setPosition(Point2DLatLon ll);
    QPointF scenePos();
    virtual void setHighlighted(bool h);
    virtual void setZValue(qreal z);
    virtual void setForbidHighlight(bool fh);
    virtual void setEditable(bool ed);
    virtual void removeFromScene();
    virtual void updateGraphics();
    virtual ItemType getType() {return ITEM_WAYPOINT;}
    void setIgnoreEvent(bool ignore);
    bool isMoving() {return moving;}

signals:
    void waypointMoved(Point2DLatLon latlon_pos);
    void waypointMoveFinished(Point2DLatLon latlon_pos);


private:
    void init();
    QString name;
    GraphicsPoint * point;
    QGraphicsTextItem* graphics_text;
    shared_ptr<Waypoint> waypoint;
    int altitude;
    bool highlighted;
    bool moving;
};

#endif // WAYPOINTITEM_H
