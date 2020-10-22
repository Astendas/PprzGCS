#include "point2dpseudomercator.h"
#include "point2dtile.h"

constexpr double EXTENT = 20037508.342789244;

const std::string Point2DPseudoMercator::EPSG = "EPSG:3857";
//EPSG:900913

Point2DPseudoMercator::Point2DPseudoMercator(double x, double y): _x(x), _y(y)
{
}

Point2DPseudoMercator::Point2DPseudoMercator(Point2DTile pt) {
    _x = (2*pt.x()/(1<<pt.zoom()) - 1) * EXTENT;
    _y = -(2*pt.y()/(1<<pt.zoom()) - 1) * EXTENT;
}

Point2DTile Point2DPseudoMercator::toTile(int zoom) {
    double xTile = ((1<<zoom)*(_x/EXTENT+ 1))/2.;
    double yTile = ((1<<zoom)*(-_y/EXTENT+ 1))/2.;
    return Point2DTile(xTile, yTile, zoom);
}
