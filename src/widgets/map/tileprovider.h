#ifndef OSMTILEPROVIDER_H
#define OSMTILEPROVIDER_H

#include <tuple>
#include <map>
#include "tileitem.h"
#include <QObject>
#include <QMap>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkDiskCache>
#include "point2dlatlon.h"
#include "point2dtile.h"
#include "tileproviderconfig.h"
#include <memory>

#define URL_MAX_LEN 350

enum TileSource {
    GOOGLE,
    OSM_CLASSIC,
    OSM_STAMEN,
    TERRAIN,
    HIKING,
    IGN,
    ICAO,
    FRANCE_DRONE_RESTRICTIONS
};

class TileProvider : public QObject
{
    Q_OBJECT
public:
    explicit TileProvider(TileProviderConfig config, int z = 0, int tileDisplaySize = 0, QObject *parent = nullptr);
    void fetch_tile(Point2DTile t, Point2DTile tObj);

    ///
    /// \brief getTile create tiles if they do not exist yet
    /// \return tile at the specified position
    ///
    TileItem* getTile(Point2DTile);

    ///
    /// \brief getValidTile return the closer valid tile : between zoomMin et zoomMax
    /// \param p
    /// \return
    ///
    TileItem* getValidTile(Point2DTile p);

    void setZoomLevel(int z);
    int zValue() {return z_value;}
    void setZValue(int z);
    qreal opacity() {return alpha;}
    void setopacity(qreal a);
    bool isVisible() {return visibility;}
    void setVisible(bool v);


    TileProviderConfig getConfig() {return config;}

signals:
    // tileReady is the tile loaded in memory, tileObj is the one to actually display
    void displayTile(TileItem* tileReady, TileItem* tileObj);

private slots:
    void handleReply(QNetworkReply *reply);

private:
    void downloadTile(TileItem* tile, TileItem* tileObj);
    void sendTile(TileItem* tileReady, TileItem* tileObj);
    std::string tilePath(Point2DTile);
    QUrl tileUrl(Point2DTile);
    bool load_tile_from_disk(TileItem*);

    TileProviderConfig config;
    int z_value;
    qreal alpha;
    bool visibility;

    /// All displayed tiles must have the same size across tilesProviders
    /// for the coordinates to be aligned
    int tileDisplaySize;

    TileItem* motherTile;
    QNetworkAccessManager* manager;
    QNetworkDiskCache* diskCache;
};

#endif // OSMTILEPROVIDER_H
