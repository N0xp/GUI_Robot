#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QPointF>
#include <QLineF>
#include <QString>
#include <QVector>
#include <cmath>

namespace Geometry {

// Point in meters
struct Point {
    double x; // meters
    double y; // meters

    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}

    QPointF toQPointF() const { return QPointF(x, y); }
    static Point fromQPointF(const QPointF& p) { return Point(p.x(), p.y()); }

    double distanceTo(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// Line segment representing a wall
struct Line {
    Point start;
    Point end;

    Line() = default;
    Line(const Point& s, const Point& e) : start(s), end(e) {}

    double length() const {
        return start.distanceTo(end);
    }

    QLineF toQLineF() const {
        return QLineF(start.toQPointF(), end.toQPointF());
    }

    // Get midpoint
    Point midpoint() const {
        return Point((start.x + end.x) / 2.0, (start.y + end.y) / 2.0);
    }

    // Calculate angle of line (in radians, 0 = horizontal right)
    double angle() const {
        double dx = end.x - start.x;
        double dy = end.y - start.y;
        return std::atan2(dy, dx);
    }

    // Calculate angle in degrees
    double angleDegrees() const {
        return angle() * 180.0 / M_PI;
    }

    // Calculate distance from a point to this line
    double distanceToPoint(const Point& p) const;
};

// Reference point (for robot position reference)
struct ReferencePoint {
    Point position;
    QString name;
    double heading;  // Optional heading reference
    bool hasHeading; // Whether heading is set

    ReferencePoint(const Point& pos = Point(), const QString& n = "Ref",
                   double h = 0.0, bool hasH = false)
        : position(pos), name(n), heading(h), hasHeading(hasH) {}
};

// Robot shape types
enum class RobotShape {
    Rectangle,
    Square,
    Triangle
};

// Robot pose (position + orientation)
struct RobotPose {
    Point position;
    double heading; // radians, 0 = pointing right
    RobotShape shape;
    double width;  // meters
    double length; // meters (ignored for square)

    RobotPose() : heading(0.0), shape(RobotShape::Rectangle),
                  width(0.6), length(0.8) {} // Default FRC robot size
};

// Waypoint in a path
struct Waypoint {
    Point position;
    double heading; // radians
    double velocity; // m/s

    Waypoint(const Point& pos = Point(), double h = 0.0, double v = 1.0)
        : position(pos), heading(h), velocity(v) {}
};

} // namespace Geometry

#endif // GEOMETRY_H
