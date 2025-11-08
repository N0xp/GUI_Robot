#include "Geometry.h"
#include <cmath>
#include <algorithm>

namespace Geometry {

double Line::distanceToPoint(const Point& p) const {
    // Vector from start to end
    double dx = end.x - start.x;
    double dy = end.y - start.y;

    // If the line is actually a point
    double lineLength = length();
    if (lineLength < 1e-6) {
        return start.distanceTo(p);
    }

    // Calculate the parameter t for the projection of point p onto the line
    // t = ((p - start) · (end - start)) / ||end - start||²
    double t = ((p.x - start.x) * dx + (p.y - start.y) * dy) / (lineLength * lineLength);

    // Clamp t to [0, 1] to keep the point on the line segment
    t = std::max(0.0, std::min(1.0, t));

    // Find the closest point on the line segment
    Point closest;
    closest.x = start.x + t * dx;
    closest.y = start.y + t * dy;

    // Return distance from p to the closest point
    return p.distanceTo(closest);
}

} // namespace Geometry
