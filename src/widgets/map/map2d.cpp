#include "map2d.h"
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <iostream>
#include "point2dlatlon.h"
#include "point2dtile.h"
#include <QtXml>
#include <QFile>
#include "utils.h"
#include "maputils.h"
#include "coordinatestransform.h"
#include <QApplication>
#include "gcs_utils.h"

Map2D::Map2D(QWidget *parent) : QGraphicsView(parent),
    numericZoom(0.0), _zoom(5.0), tile_size(256), minZoom(0.0), maxZoom(25.0), wheelAccumulator(0)
{
    auto settings = getAppSettings();
    QString configFile = user_or_app_path("tile_sources.xml");
    auto configs = loadConfig(configFile);
    for(auto c:configs) {
        auto providerName = c->name;
        if(tile_providers.size() == 0) {
            // the tile size choosen will be the one of the first tile provider.
            tile_size = c->tileSize;
        }
        QString tiles_path = settings.value("map/tiles_path").toString();
        int z =configs.count() - c->initial_rank;
        auto tp = new TileProvider(c, z, tile_size, tiles_path, this);
        tp->setOpacity(1);
        tp->setVisible(false);
        connect(tp, &TileProvider::displayTile, this, &Map2D::handleTile);
        tile_providers[providerName] = tp;
    }

    qreal maxxy = tile_size*pow(2, maxZoom);
    _scene = new MapScene(-maxxy, -maxxy, 2*tile_size*maxxy, 2*tile_size*maxxy, this);
    setScene(_scene);

    setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setBackgroundBrush(QBrush(QColor(0x151515)));

    Point2DLatLon initLatLon(43.462344,1.273044);
}

Map2D::~Map2D() {
    for(auto tp:qAsConst(tile_providers)) {
        tp->removeFromScene(_scene);
    }
}

void Map2D::toggleTileProvider(QString providerName, bool enable) {
    if(tile_providers.contains(providerName)) {
        tile_providers[providerName]->setVisible(enable);
    }
}

void Map2D::setLayerOpacity(QString providerName, qreal opacity) {
    if(tile_providers.contains(providerName)) {
        tile_providers[providerName]->setOpacity(opacity);
    }
}

void Map2D::setLayerZ(QString providerName, int z) {
    if(tile_providers.contains(providerName)) {
        tile_providers[providerName]->setZValue(z);
    }
}

void Map2D::centerLatLon(Point2DLatLon latLon) {
    centerOn(scenePoint(latLon, zoomLevel(_zoom), tile_size));
    updateTiles();
}

/**
 * @brief Map2D::zoomBox
 * @param nw
 * @param se
 * @return max zoom fitting nw and se.
 */
double Map2D::zoomBox(Point2DLatLon nw, Point2DLatLon se) {
    double targetZoom = 0;
    while(targetZoom < 25) {
        Point2DTile pnw(nw, zoomLevel(targetZoom));
        Point2DTile pse(se, zoomLevel(targetZoom));
        double sf = pow(2, targetZoom - zoomLevel(targetZoom));
        int dx = (pse.x() - pnw.x()) * tileSize() / sf;
        int dy = (pse.y() - pnw.y()) * tileSize() / sf;

        if(dx >= viewport()->rect().width() || dy >= viewport()->rect().height()) {
            break;
        }
        targetZoom += 0.5;
    }
    return targetZoom;
}

QList<TileProviderConfig*> Map2D::loadConfig(QString filename) {
    QList<TileProviderConfig*> configs;

    QDomDocument xmlLayout;
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly)) {
        std::cout << "Error reading file " << filename.toStdString() << "!" << std::endl;
    }
    xmlLayout.setContent(&f);
    f.close();

    QDomElement root = xmlLayout.documentElement();
    QString rootTag = root.tagName();
    if (rootTag != "sources") {
        std::cout << "Root tag expected to be \"sources\". Is this a tileSource file ?" << std::endl;
    }

    for(int i=0; i<root.childNodes().length(); i++) {
        if(root.childNodes().item(i).isElement()) {
            QDomElement ele = root.childNodes().item(i).toElement();
            auto tpc = new TileProviderConfig(ele);
            tpc->initial_rank = i;
            configs.append(tpc);
        }
    }
    return configs;
}

void Map2D::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    updateTiles();
}

void Map2D::wheelEvent(QWheelEvent* event) {
    setResizeAnchor(QGraphicsView::NoAnchor);

    wheelAccumulator += event->angleDelta().y();
    event->accept();

    if(qAbs(wheelAccumulator) < 120) {
        return;
    }

    double new_zoom;
    if(wheelAccumulator > 0) {
        new_zoom = _zoom + 0.5;
    } else {
        new_zoom = _zoom - 0.5;
    }
    new_zoom = round(new_zoom*2)/2.0;
    zoomCentered(new_zoom, event->pos());
    wheelAccumulator = 0;
}

void Map2D::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Plus) {
        zoomCentered(_zoom + 0.5, QPoint(width()/2, height()/2));
        event->accept();
    } else if (event->key() == Qt::Key_Minus) {
        zoomCentered(_zoom - 0.5, QPoint(width()/2, height()/2));
        event->accept();
    } else {
        event->ignore();
    }
}

void Map2D::setZoom(double z) {
    zoomCentered(z, QPoint(width()/2, height()/2));
}

void Map2D::zoomCentered(double z, QPoint eventPos) {
    int curZoom = zoomLevel(_zoom);
    _zoom = std::clamp(z, minZoom, maxZoom);

    // save initial numericZoom
    double  numZoomIni = numericZoom;

    // for tileProvider in tileProviders...
    int nextZoomLevel = zoomLevel(_zoom);
    numericZoom = _zoom - nextZoomLevel;

    double scaleFactor = pow(2, numericZoom) / pow(2, numZoomIni);

    // mouse pos in scene
    QPointF oldPos = mapToScene(eventPos);
    // lat lon point pointed by the mouse (at the current zoomLevel)
    auto poi = CoordinatesTransform::get()->wgs84_from_scene(oldPos, curZoom, tile_size);

    scale(scaleFactor, scaleFactor);    // apply scale
    // mouse pos in scene after scale
    QPointF newPos = mapToScene(eventPos);
    // position of the poi in scene coordinates for the new zoom
    QPointF poi_scene = scenePoint(poi, nextZoomLevel, tile_size);

    QPointF delta = newPos - poi_scene;
    translate(delta.x(), delta.y());

    for(auto elt: qAsConst(tile_providers)) {
        elt->setZoomLevel(nextZoomLevel);
    }

    updateTiles();

}

void Map2D::mouseMoveEvent(QMouseEvent *event) {
    QGraphicsView::mouseMoveEvent(event);
    if(event->buttons() & mouse_load_tiles_mask) {
        updateTiles();
    }
}

void Map2D::updateTiles() {
    QPointF center = mapToScene(QPoint(width()/2,height()/2));

    int xCenter = static_cast<int>(center.x()/tile_size);
    int yCenter = static_cast<int>(center.y()/tile_size);
    int N = ceil(std::max(width(), height()) / (2.0 * tile_size)) + 2;

    for(auto elt=tile_providers.begin(); elt!=tile_providers.end(); ++elt) {
        TileProvider* tileProvider = elt.value();
        if(tileProvider->isVisible()) {

            Point2DTile coor = Point2DTile(xCenter, yCenter, zoomLevel(_zoom));
            tileProvider->fetch_tile(coor, coor);

            for(int n=0; n<=N; n++) {
                for(int i=-n; i<=n; i++) {
                    coor = Point2DTile(xCenter + i, yCenter + n, zoomLevel(_zoom));
                    tileProvider->fetch_tile(coor, coor);
                    coor = Point2DTile(xCenter + i, yCenter - n, zoomLevel(_zoom));
                    tileProvider->fetch_tile(coor, coor);
                }
                for(int j=1-n; j<n; j++) {
                    coor = Point2DTile(xCenter + n, yCenter + j, zoomLevel(_zoom));
                    tileProvider->fetch_tile(coor, coor);
                    coor = Point2DTile(xCenter - n, yCenter + j, zoomLevel(_zoom));
                    tileProvider->fetch_tile(coor, coor);
                }
            }
        }
    }
}

void Map2D::getViewPoints(Point2DLatLon& nw, Point2DLatLon& se) {
    QPointF top_left = mapToScene(QPoint(0,0));
    QPointF bottom_right = mapToScene(QPoint(width(),height()));
    nw = CoordinatesTransform::get()->wgs84_from_scene(top_left, zoomLevel(zoom()), tile_size);
    se = CoordinatesTransform::get()->wgs84_from_scene(bottom_right, zoomLevel(zoom()), tile_size);
}


void Map2D::handleTile(TileItem* tileReady, TileItem* tileObj) {
    if(tileReady->hasData()){
        if(!tileObj->isInScene()) {    // Not in scene, so lets add it
            _scene->addItem(tileObj);
            tileObj->setInScene(true);
        }
        if(!tileObj->isVisible()) {    // in scene but hidden, lets show it. TODO: what if this slot is called just atfer a zoom change ?
            if(tileObj->coordinates().zoom() == zoomLevel(_zoom)) {
                tileObj->show();
            }
        }

        QPointF pos = QPointF(
            tile_size*(tileObj->coordinates().x()),
            tile_size*(tileObj->coordinates().y())
        );
        tileObj->setPos(pos);
    } else {
        std::cout << "WHAAAAT ? Why I am receiving this signal but I have no data ?" << std::endl;
    }
}

QList<TileProvider*> Map2D::tileProviders() {
    auto tps = tile_providers.values();
    std::sort(tps.begin(), tps.end(),
        [](TileProvider* ltp, TileProvider* rtp) {
            return ltp->config()->initial_rank < rtp->config()->initial_rank;
    });
    return tps;
}

Point2DLatLon Map2D::latlonFromView(QPoint viewPos, int zoom) {
    return CoordinatesTransform::get()->wgs84_from_scene(mapToScene(viewPos), zoom, tile_size);
}
