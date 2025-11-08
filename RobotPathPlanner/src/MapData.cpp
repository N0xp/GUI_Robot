#include "MapData.h"
#include <QFile>
#include <QJsonDocument>
#include <limits>

MapData::MapData()
    : name("Untitled Map")
    , origin(0.0, 0.0)
    , gridSize(1.0) // 1 meter grid by default
{
}

void MapData::addLine(const Geometry::Line& line) {
    lines.append(line);
}

void MapData::removeLine(int index) {
    if (index >= 0 && index < lines.size()) {
        lines.remove(index);
    }
}

void MapData::clear() {
    lines.clear();
    referencePoints.clear();
}

void MapData::addReferencePoint(const Geometry::ReferencePoint& refPoint) {
    referencePoints.append(refPoint);
}

void MapData::removeReferencePoint(int index) {
    if (index >= 0 && index < referencePoints.size()) {
        referencePoints.remove(index);
    }
}

int MapData::findClosestReferencePoint(const Geometry::Point& point, double maxDistance) const {
    int closestIndex = -1;
    double closestDist = maxDistance;

    for (int i = 0; i < referencePoints.size(); ++i) {
        double dist = referencePoints[i].position.distanceTo(point);
        if (dist < closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }

    return closestIndex;
}

QJsonObject MapData::toJson() const {
    QJsonObject json;

    json["name"] = name;
    // Origin is always (0,0) - it's the fixed global coordinate reference
    json["gridSize"] = gridSize;

    QJsonArray linesArray;
    for (const auto& line : lines) {
        linesArray.append(lineToJson(line));
    }
    json["lines"] = linesArray;

    QJsonArray refPointsArray;
    for (const auto& rp : referencePoints) {
        refPointsArray.append(refPointToJson(rp));
    }
    json["referencePoints"] = refPointsArray;

    return json;
}

bool MapData::fromJson(const QJsonObject& json) {
    if (!json.contains("name") || !json.contains("lines")) {
        return false;
    }

    name = json["name"].toString();

    // Origin is always (0,0) - it's the fixed global coordinate reference
    // Ignore any origin data from old JSON files
    origin = Geometry::Point(0.0, 0.0);

    if (json.contains("gridSize")) {
        gridSize = json["gridSize"].toDouble();
    }

    lines.clear();
    QJsonArray linesArray = json["lines"].toArray();
    for (const auto& lineValue : linesArray) {
        lines.append(lineFromJson(lineValue.toObject()));
    }

    referencePoints.clear();
    if (json.contains("referencePoints")) {
        QJsonArray refPointsArray = json["referencePoints"].toArray();
        for (const auto& rpValue : refPointsArray) {
            referencePoints.append(refPointFromJson(rpValue.toObject()));
        }
    }

    return true;
}

bool MapData::saveToFile(const QString& filepath) const {
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool MapData::loadFromFile(const QString& filepath) {
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

int MapData::findClosestLine(const Geometry::Point& point, double maxDistance) const {
    int closestIndex = -1;
    double closestDist = maxDistance;

    for (int i = 0; i < lines.size(); ++i) {
        double dist = lines[i].distanceToPoint(point);
        if (dist < closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }

    return closestIndex;
}

double MapData::distanceToNearestWall(const Geometry::Point& point) const {
    if (lines.isEmpty()) {
        return std::numeric_limits<double>::infinity();
    }

    double minDist = std::numeric_limits<double>::infinity();
    for (const auto& line : lines) {
        double dist = line.distanceToPoint(point);
        if (dist < minDist) {
            minDist = dist;
        }
    }

    return minDist;
}

QJsonObject MapData::pointToJson(const Geometry::Point& p) {
    QJsonObject json;
    json["x"] = p.x;
    json["y"] = p.y;
    return json;
}

Geometry::Point MapData::pointFromJson(const QJsonObject& json) {
    return Geometry::Point(
        json["x"].toDouble(),
        json["y"].toDouble()
    );
}

QJsonObject MapData::lineToJson(const Geometry::Line& line) {
    QJsonObject json;
    json["start"] = pointToJson(line.start);
    json["end"] = pointToJson(line.end);
    json["length"] = line.length();
    json["angle"] = line.angleDegrees();  // Add angle to JSON output
    return json;
}

Geometry::Line MapData::lineFromJson(const QJsonObject& json) {
    return Geometry::Line(
        pointFromJson(json["start"].toObject()),
        pointFromJson(json["end"].toObject())
    );
}

QJsonObject MapData::refPointToJson(const Geometry::ReferencePoint& rp) {
    QJsonObject json;
    json["position"] = pointToJson(rp.position);
    json["name"] = rp.name;
    if (rp.hasHeading) {
        json["heading"] = rp.heading * 180.0 / M_PI;  // Convert to degrees for JSON
        json["hasHeading"] = true;
    } else {
        json["hasHeading"] = false;
    }
    return json;
}

Geometry::ReferencePoint MapData::refPointFromJson(const QJsonObject& json) {
    Geometry::Point pos = pointFromJson(json["position"].toObject());
    QString name = json["name"].toString();
    bool hasHeading = json["hasHeading"].toBool();
    double heading = 0.0;
    if (hasHeading && json.contains("heading")) {
        heading = json["heading"].toDouble() * M_PI / 180.0;  // Convert degrees to radians
    }
    return Geometry::ReferencePoint(pos, name, heading, hasHeading);
}
