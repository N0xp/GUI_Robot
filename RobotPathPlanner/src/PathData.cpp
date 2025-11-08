#include "PathData.h"
#include <QFile>
#include <QJsonDocument>

// PathData implementation
PathData::PathData(const QString& name)
    : name(name)
    , color(Qt::blue)
    , visible(true)
{
}

void PathData::addWaypoint(const Geometry::Waypoint& wp) {
    waypoints.append(wp);
}

void PathData::insertWaypoint(int index, const Geometry::Waypoint& wp) {
    if (index >= 0 && index <= waypoints.size()) {
        waypoints.insert(index, wp);
    }
}

void PathData::removeWaypoint(int index) {
    if (index >= 0 && index < waypoints.size()) {
        waypoints.remove(index);
    }
}

void PathData::updateWaypoint(int index, const Geometry::Waypoint& wp) {
    if (index >= 0 && index < waypoints.size()) {
        waypoints[index] = wp;
    }
}

void PathData::clear() {
    waypoints.clear();
}

double PathData::totalLength() const {
    double length = 0.0;
    for (int i = 1; i < waypoints.size(); ++i) {
        length += waypoints[i - 1].position.distanceTo(waypoints[i].position);
    }
    return length;
}

QJsonObject PathData::toJson() const {
    QJsonObject json;

    json["name"] = name;
    json["color"] = color.name();
    json["visible"] = visible;

    QJsonArray waypointsArray;
    for (const auto& wp : waypoints) {
        waypointsArray.append(waypointToJson(wp));
    }
    json["waypoints"] = waypointsArray;

    return json;
}

bool PathData::fromJson(const QJsonObject& json) {
    if (!json.contains("name") || !json.contains("waypoints")) {
        return false;
    }

    name = json["name"].toString();

    if (json.contains("color")) {
        color = QColor(json["color"].toString());
    }

    if (json.contains("visible")) {
        visible = json["visible"].toBool();
    }

    waypoints.clear();
    QJsonArray waypointsArray = json["waypoints"].toArray();
    for (const auto& wpValue : waypointsArray) {
        waypoints.append(waypointFromJson(wpValue.toObject()));
    }

    return true;
}

bool PathData::saveToFile(const QString& filepath) const {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool PathData::loadFromFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    return fromJson(doc.object());
}

QJsonObject PathData::waypointToJson(const Geometry::Waypoint& wp) {
    QJsonObject json;
    json["x"] = wp.position.x;
    json["y"] = wp.position.y;
    json["theta_rad"] = wp.heading;             // Primary: radians (for robot code)
    json["theta"] = wp.heading * 180.0 / M_PI;  // Secondary: degrees (for human readability)
    json["velocity"] = wp.velocity;
    return json;
}

Geometry::Waypoint PathData::waypointFromJson(const QJsonObject& json) {
    Geometry::Point pos(json["x"].toDouble(), json["y"].toDouble());

    // Try to read heading from multiple fields for backwards compatibility
    double heading = 0.0;
    if (json.contains("theta_rad")) {
        heading = json["theta_rad"].toDouble();  // Primary: radians
    } else if (json.contains("theta")) {
        heading = json["theta"].toDouble() * M_PI / 180.0;  // Fallback: convert degrees to radians
    } else if (json.contains("heading_deg")) {
        heading = json["heading_deg"].toDouble() * M_PI / 180.0;  // Fallback: convert degrees to radians
    } else if (json.contains("heading")) {
        heading = json["heading"].toDouble();  // Old format fallback
    }

    double velocity = json.contains("velocity") ? json["velocity"].toDouble() : 1.0;
    return Geometry::Waypoint(pos, heading, velocity);
}

// PathCollection implementation
PathCollection::PathCollection()
    : activePathIndex(-1)
{
}

void PathCollection::addPath(const PathData& path) {
    paths.append(path);
    if (activePathIndex < 0) {
        activePathIndex = 0;
    }
}

void PathCollection::removePath(int index) {
    if (index >= 0 && index < paths.size()) {
        paths.remove(index);
        if (activePathIndex >= paths.size()) {
            activePathIndex = paths.size() - 1;
        }
    }
}

PathData* PathCollection::getActivePath() {
    if (activePathIndex >= 0 && activePathIndex < paths.size()) {
        return &paths[activePathIndex];
    }
    return nullptr;
}

const PathData* PathCollection::getActivePath() const {
    if (activePathIndex >= 0 && activePathIndex < paths.size()) {
        return &paths[activePathIndex];
    }
    return nullptr;
}

QJsonObject PathCollection::toJson() const {
    QJsonObject json;

    // Metadata for mission planning
    json["version"] = "1.0";
    json["description"] = "Robot mission paths for autonomous execution";
    json["units"] = "meters and degrees";
    json["coordinate_system"] = "Standard: X-right, Y-up, Theta 0=East CCW";
    json["total_paths"] = paths.size();
    json["activePathIndex"] = activePathIndex;

    // Export all paths
    QJsonArray pathsArray;
    for (int i = 0; i < paths.size(); ++i) {
        QJsonObject pathObj = paths[i].toJson();
        pathObj["path_index"] = i;  // Add index for easy reference
        pathObj["path_length_meters"] = paths[i].totalLength();
        pathObj["waypoint_count"] = paths[i].waypoints.size();
        pathsArray.append(pathObj);
    }
    json["paths"] = pathsArray;

    return json;
}

bool PathCollection::fromJson(const QJsonObject& json) {
    if (!json.contains("paths")) {
        return false;
    }

    activePathIndex = json.contains("activePathIndex") ? json["activePathIndex"].toInt() : -1;

    paths.clear();
    QJsonArray pathsArray = json["paths"].toArray();
    for (const auto& pathValue : pathsArray) {
        PathData path;
        if (path.fromJson(pathValue.toObject())) {
            paths.append(path);
        }
    }

    // Validate activePathIndex
    if (activePathIndex >= paths.size()) {
        activePathIndex = paths.size() - 1;
    }

    return true;
}

bool PathCollection::saveToFile(const QString& filepath) const {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool PathCollection::loadFromFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    return fromJson(doc.object());
}
