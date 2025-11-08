#ifndef MAPDATA_H
#define MAPDATA_H

#include "Geometry.h"
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class MapData {
public:
    MapData();

    // Map properties
    QString name;
    Geometry::Point origin; // Reference point (0,0)
    double gridSize; // meters per grid square

    // Walls/lines
    QVector<Geometry::Line> lines;

    // Reference points
    QVector<Geometry::ReferencePoint> referencePoints;

    // Add/remove lines
    void addLine(const Geometry::Line& line);
    void removeLine(int index);
    void clear();

    // Add/remove reference points
    void addReferencePoint(const Geometry::ReferencePoint& refPoint);
    void removeReferencePoint(int index);
    int findClosestReferencePoint(const Geometry::Point& point, double maxDistance = 0.5) const;

    // JSON serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    // Save/load from file
    bool saveToFile(const QString& filepath) const;
    bool loadFromFile(const QString& filepath);

    // Find closest line to a point
    int findClosestLine(const Geometry::Point& point, double maxDistance = 0.5) const;

    // Calculate distance from a point to nearest wall
    // Returns std::numeric_limits<double>::infinity() if there are no walls in the map
    double distanceToNearestWall(const Geometry::Point& point) const;

private:
    static QJsonObject pointToJson(const Geometry::Point& p);
    static Geometry::Point pointFromJson(const QJsonObject& json);
    static QJsonObject lineToJson(const Geometry::Line& line);
    static Geometry::Line lineFromJson(const QJsonObject& json);
    static QJsonObject refPointToJson(const Geometry::ReferencePoint& rp);
    static Geometry::ReferencePoint refPointFromJson(const QJsonObject& json);
};

#endif // MAPDATA_H
