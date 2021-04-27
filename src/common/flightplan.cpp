#include "flightplan.h"
#include "iostream"
#include "gcs_utils.h"

#include <clocale>
#include "coordinatestransform.h"

using namespace std;
using namespace tinyxml2;

FlightPlan::FlightPlan(): origin()
{

}


FlightPlan::FlightPlan(QString uri) : origin()
{
    setlocale(LC_ALL, "C"); // needed for stod() to use '.' as decimal separator instead of ',' (at least in France)
    QDomDocument doc;

    if(uri.mid(0,4) == "file") {
        QString path = uri.mid(7, uri.length()-7);
        QFile f(path);
        if(!f.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Error while loading flightplan file");
        }
        doc.setContent(&f);
        f.close();
    }
        QDomElement fp_root = doc.firstChildElement( "dump" ).firstChildElement( "flight_plan" );
        name = fp_root.attribute("name");
        auto lat0 = fp_root.attribute("lat0");
        auto lon0 = fp_root.attribute("lon0");
        auto defalt = fp_root.attribute("alt");
        auto max_dist_home = fp_root.attribute("max_dist_from_home");
        auto gnd_alt = fp_root.attribute("ground_alt");
        auto sec_h = fp_root.attribute("security_height");

        defaultAlt = defalt.toDouble();
        max_dist_from_home = max_dist_home.toDouble();
        ground_alt = gnd_alt.toDouble();
        security_height = sec_h.toDouble();

        double _lat0 = parse_coordinate(lat0);
        double _lon0 = parse_coordinate(lon0);

        frame_type = Waypoint::WpFrame::UTM;
        if(fp_root.hasAttribute("wp_frame")) {
            auto f = fp_root.attribute("wp_frame");
            if(f.toUpper()  == "LTP") {
                frame_type = Waypoint::WpFrame::LTP;
            }
        }

        origin = make_shared<Waypoint>("__ORIGIN", 0, _lat0, _lon0, defaultAlt);

        auto wps = fp_root.firstChildElement("waypoints");
        parse_waypoints(wps);
        auto blks = fp_root.firstChildElement("blocks");
        parse_blocks(blks);

        auto exs = fp_root.firstChildElement("exceptions");
        if(!exs.isNull()) {
            parse_exceptions(exs);
        }

        auto vars = fp_root.firstChildElement("variables");
        if(!vars.isNull()) {
            parse_variables(vars);
        }

        auto secs = fp_root.firstChildElement("sectors");
        if(!secs.isNull()) {
            parse_sectors(secs);
        }
}

void FlightPlan::parse_exceptions(QDomElement exs) {
    for(auto ex=exs.firstChildElement();
        !ex.isNull();
        ex=ex.nextSiblingElement()) {
        auto cond = ex.attribute("cond");
        auto deroute = ex.attribute("deroute");

        auto e = make_shared<Exception>();
        e->cond = cond;
        e->deroute = deroute;

        exceptions.push_back(e);
    }
}

void FlightPlan::parse_variables(QDomElement vars) {
    for(auto ele=vars.firstChildElement(); !ele.isNull(); ele=ele.nextSiblingElement()) {
        shared_ptr<Variable> var;
        if(ele.tagName() == "variable") {
            var = make_shared<Variable>(Variable::VARIABLE, ele.attribute("var"));
        } else if (ele.tagName() == "abi_binding") {
            var = make_shared<Variable>(Variable::ABI_BINDING, ele.attribute("name"));
        } else {
            throw std::runtime_error("");
        }

        auto attr = ele.attributes();
        for(int i=0; i < attr.count(); i++) {
            auto att = attr.item(i).toAttr();
            var->attributes[att.name()] = att.value();
        }

        variables.push_back(var);
    }
}

void FlightPlan::parse_sectors(QDomElement secs) {

    for(auto ele=secs.firstChildElement(); !ele.isNull(); ele=ele.nextSiblingElement()) {
        if(ele.tagName() == "sector") {
            // It's a sector!

            auto sec_name = ele.attribute("name");
            auto color = ele.attribute("color");
            optional<QString> sec_color = nullopt;
            if(color != nullptr) {
                sec_color = color;
            }

            auto type = ele.attribute("type", "");
            Sector::Type t = Sector::DYNAMIC;
            if(type == "static") {
                t = Sector::STATIC;
            }
            vector<shared_ptr<Waypoint>> corners;
            for(auto corner=ele.firstChildElement(); !corner.isNull(); corner=corner.nextSiblingElement()) {
                auto wp = getWaypoint(corner.attribute("name"));
                corners.push_back(wp);
            }
            auto sec = make_shared<Sector>(corners, sec_name, t, sec_color);
            sectors.push_back(sec);
        } else if (ele.tagName() == "kml") {
            //It's a KML!
            throw std::runtime_error("Sector from KML not implemented!");
        } else {
            throw std::runtime_error("");
        }
    }
}

void FlightPlan::parse_waypoints(QDomElement wps) {
    uint8_t wp_id = 1;
    for(auto wp=wps.firstChildElement(); !wp.isNull(); wp=wp.nextSiblingElement()) {
        auto waypoint = make_shared<Waypoint>(wp, wp_id, origin, defaultAlt, frame_type);
        waypoints.push_back(waypoint);
        ++wp_id;
    }
}

void FlightPlan::parse_blocks(QDomElement blks) {

    for(auto blk=blks.firstChildElement(); !blk.isNull(); blk=blk.nextSiblingElement()) {
        auto block = make_shared<Block>(blk);
        blocks.push_back(block);
    }

    std::sort(blocks.begin(), blocks.end(),
              [](auto lblock, auto rblock) {
                    return lblock->getNo() < rblock->getNo();
    });
}

shared_ptr<Waypoint> FlightPlan::getWaypoint(uint8_t id) {
    for(auto& wp: waypoints) {
        if(wp->getId() == id) {
            return wp;
        }
    }
    throw runtime_error("No waypoint with id " + to_string(id));
}

shared_ptr<Waypoint> FlightPlan::getWaypoint(QString name) {
    for(auto& wp: waypoints) {
        if(wp->getName() == name) {
            return wp;
        }
    }
    throw runtime_error("No waypoint with name " + name.toStdString());
}

shared_ptr<Block> FlightPlan::getBlock(uint8_t no) {
    for(auto& bl: blocks) {
        if(bl->getNo() == no) {
            return bl;
        }
    }
    throw runtime_error("No block with no " + to_string(no));
}

vector<shared_ptr<BlockGroup>> FlightPlan::getGroups()
{

    std::map<QString, shared_ptr<BlockGroup>> groups_map;

    for(auto b: blocks) {
        if(groups_map.find(b->getGroup()) == groups_map.end()) {
            groups_map[b->getGroup()] = make_shared<BlockGroup>();
            groups_map[b->getGroup()]->group_name = b->getGroup();
        }
        groups_map[b->getGroup()]->blocks.push_back(b);
    }


    vector<shared_ptr<BlockGroup>> groups;

    for( auto it = groups_map.begin(); it != groups_map.end(); ++it ) {
        groups.push_back( it->second );
    }

    sort(groups.begin(), groups.end(),
        [](shared_ptr<BlockGroup> lb, shared_ptr<BlockGroup> rb) {
                return lb->group_name < rb->group_name;
    });

    return groups;

}

std::tuple<Point2DLatLon, Point2DLatLon> FlightPlan::boundingBox() {
    double min_lat = 90;
    double min_lon = 180;
    double max_lat = -90;
    double max_lon = -180;
    for(auto &wp: waypoints) {
        min_lat = min(min_lat, wp->getLat());
        max_lat = max(max_lat, wp->getLat());
        min_lon = min(min_lon, wp->getLon());
        max_lon = max(max_lon, wp->getLon());
    }

    //NW, SE
    return make_tuple(Point2DLatLon(max_lat, min_lon), Point2DLatLon(min_lat, max_lon));
}

std::tuple<Point2DLatLon, Point2DLatLon> FlightPlan::boundingBoxWith(Point2DLatLon pt) {
    auto [nw, se] = boundingBox();

    double min_lat = min(pt.lat(), se.lat());
    double max_lat = max(pt.lat(), nw.lat());
    double min_lon = min(pt.lon(), nw.lat());
    double max_lon = max(pt.lon(), se.lon());

    return make_tuple(Point2DLatLon(max_lat, min_lon), Point2DLatLon(min_lat, max_lon));
}
