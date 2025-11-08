#ifndef PATHDATA_H
#define PATHDATA_H

#include "Geometry.h"
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QColor>

class PathData {
public:
    PathData(const QString& name = "Untitled Path");

    // Path properties
    QString name;
    QColor color;
    bool visible;
    QVector<Geometry::Waypoint> waypoints;

    // Add/remove waypoints
    void addWaypoint(const Geometry::Waypoint& wp);
    void insertWaypoint(int index, const Geometry::Waypoint& wp);
    void removeWaypoint(int index);
    void updateWaypoint(int index, const Geometry::Waypoint& wp);
    void clear();

    // Get total path length
    double totalLength() const;

    // JSON serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    // Save/load from file
    bool saveToFile(const QString& filepath) const;
    bool loadFromFile(const QString& filepath);

private:
    static QJsonObject waypointToJson(const Geometry::Waypoint& wp);
    static Geometry::Waypoint waypointFromJson(const QJsonObject& json);
};

// Container for multiple paths
class PathCollection {
public:
    PathCollection();

    QVector<PathData> paths;
    int activePathIndex;

    void addPath(const PathData& path);
    void removePath(int index);
    PathData* getActivePath();
    const PathData* getActivePath() const;

    // JSON serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& json);

    // Save/load all paths
    bool saveToFile(const QString& filepath) const;
    bool loadFromFile(const QString& filepath);
};

#endif // PATHDATA_H
