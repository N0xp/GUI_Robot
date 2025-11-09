#include "MapCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainterPath>
#include <QInputDialog>
#include <QPalette>
#include <cmath>
#include <limits>

// Named constants for magic numbers
namespace {
    // Snap and proximity thresholds (in meters)
    constexpr double DEFAULT_SNAP_DISTANCE = 0.2;          // 20cm snap radius
    constexpr double WAYPOINT_CLICK_THRESHOLD = 0.3;       // 30cm threshold for waypoint selection
    constexpr double WAYPOINT_HANDLE_THRESHOLD = 0.15;     // 15cm threshold for heading handle
    constexpr double ROBOT_PROXIMITY_THRESHOLD = 0.5;      // 50cm threshold for robot selection
    constexpr double LINE_PROXIMITY_THRESHOLD = 0.2;       // 20cm threshold for line selection

    // View and scale defaults
    constexpr double DEFAULT_SCALE = 50.0;                 // 50 pixels per meter
    constexpr double MIN_SCALE = 5.0;                      // Minimum zoom level
    constexpr double MAX_SCALE = 500.0;                    // Maximum zoom level

    // Dark theme palette
    const QColor CANVAS_BG_TOP(250, 250, 252);
    const QColor CANVAS_BG_BOTTOM(245, 245, 248);
    const QColor PANEL_BG(22, 26, 40, 235);
    const QColor PANEL_BORDER(104, 128, 180);
    const QColor TEXT_PRIMARY(240, 244, 255);
    const QColor TEXT_MUTED(180, 192, 210);
    const QColor GRID_MAJOR_COLOR(110, 130, 170);
    const QColor GRID_MINOR_COLOR(60, 75, 105);
    const QColor ACCENT_ORANGE(255, 140, 70);
    const QColor ACCENT_TEAL(0, 220, 190);
}

MapCanvas::MapCanvas(QWidget* parent)
    : QWidget(parent)
    , m_mapData(nullptr)
    , m_pathCollection(nullptr)
    , m_viewOffset(0, 0)
    , m_scale(DEFAULT_SCALE)
    , m_minScale(MIN_SCALE)
    , m_maxScale(MAX_SCALE)
    , m_primaryRobotIndex(0)
    , m_currentTool(Tool::None)
    , m_isDrawing(false)
    , m_waitingForSecondClick(false)
    , m_constrainedDrawing(false)
    , m_constrainedDistance(1.0)
    , m_constrainedAngle(0.0)
    , m_lastDrawPoint(0, 0)
    , m_cursorWorldPos(0, 0)
    , m_measuring(false)
    , m_measureLocked(false)
    , m_measureSnappedToLine(false)
    , m_measureSnappedToRobot(false)
    , m_measureSnappedLineIndex(-1)
    , m_measureSnappedRobotIndex(-1)
    , m_showGrid(true)
    , m_showDimensions(true)
    , m_showRobot(true)
    , m_snapToPoints(true)
    , m_snapDistance(DEFAULT_SNAP_DISTANCE)
    , m_isDraggingRobot(false)
    , m_draggedRobotIndex(-1)
    , m_isDraggingWaypoint(false)
    , m_draggedPathIndex(-1)
    , m_draggedWaypointIndex(-1)
    , m_isEditingHeading(false)
    , m_editingPathIndex(-1)
    , m_editingWaypointIndex(-1)
    , m_selectedLineIndex(-1)
    , m_isDraggingLineEndpoint(false)
    , m_draggingLineStart(false)
    , m_selectedPathIndex(-1)
    , m_selectedWaypointIndex(-1)
    , m_isPanning(false)
    , m_fixedSnapEnabled(true)
    , m_fixedLengthStep(0.05)
    , m_fixedAngleStep(5.0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 400);

    // Add default robot
    Geometry::RobotPose defaultRobot;
    m_robots.append(defaultRobot);
}

void MapCanvas::setMapData(MapData* mapData) {
    m_mapData = mapData;
    update();
}

void MapCanvas::setPathCollection(PathCollection* paths) {
    m_pathCollection = paths;
    update();
}

void MapCanvas::setRobotPose(const Geometry::RobotPose& pose) {
    // Update the primary robot
    if (m_primaryRobotIndex >= 0 && m_primaryRobotIndex < m_robots.size()) {
        m_robots[m_primaryRobotIndex] = pose;
    }
    update();
}

Geometry::RobotPose MapCanvas::getCurrentPose() const {
    if (m_primaryRobotIndex >= 0 && m_primaryRobotIndex < m_robots.size()) {
        return m_robots[m_primaryRobotIndex];
    }
    return Geometry::RobotPose();
}

void MapCanvas::addRobot(const Geometry::RobotPose& pose) {
    m_robots.append(pose);
    update();
}

void MapCanvas::removeRobot(int index) {
    if (index >= 0 && index < m_robots.size()) {
        m_robots.remove(index);
        if (m_primaryRobotIndex >= m_robots.size()) {
            m_primaryRobotIndex = m_robots.size() - 1;
        }
        update();
    }
}

void MapCanvas::clearRobots() {
    m_robots.clear();
    m_primaryRobotIndex = -1;
    update();
}

void MapCanvas::updateRobotPose(int index, const Geometry::RobotPose& pose) {
    if (index >= 0 && index < m_robots.size()) {
        m_robots[index] = pose;
        update();
    }
}

void MapCanvas::setTool(Tool tool) {
    m_currentTool = tool;
    m_isDrawing = false;
    m_waitingForSecondClick = false;
    m_measuring = false;

    // Update cursor based on tool
    switch (tool) {
        case Tool::DrawLine:
        case Tool::DrawPath:
            setCursor(Qt::CrossCursor);
            break;
        case Tool::Pan:
            setCursor(Qt::OpenHandCursor);
            break;
        case Tool::Measure:
            setCursor(Qt::CrossCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
}

void MapCanvas::zoomIn() {
    m_scale = qMin(m_scale * 1.2, m_maxScale);
    update();
}

void MapCanvas::zoomOut() {
    m_scale = qMax(m_scale / 1.2, m_minScale);
    update();
}

void MapCanvas::resetView() {
    m_scale = DEFAULT_SCALE;
    m_viewOffset = QPointF(width() / 2.0, height() / 2.0);
    update();
}

void MapCanvas::fitToView() {
    if (!m_mapData || m_mapData->lines.isEmpty()) {
        resetView();
        return;
    }

    // Calculate bounding box
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    for (const auto& line : m_mapData->lines) {
        minX = qMin(minX, qMin(line.start.x, line.end.x));
        minY = qMin(minY, qMin(line.start.y, line.end.y));
        maxX = qMax(maxX, qMax(line.start.x, line.end.x));
        maxY = qMax(maxY, qMax(line.start.y, line.end.y));
    }

    double rangeX = maxX - minX;
    double rangeY = maxY - minY;
    double centerX = (minX + maxX) / 2.0;
    double centerY = (minY + maxY) / 2.0;

    // Prevent division by zero when all lines are at the same point
    const double MIN_RANGE = 1e-6;
    if (rangeX < MIN_RANGE || rangeY < MIN_RANGE) {
        // All map elements are essentially at a single point
        m_scale = DEFAULT_SCALE;
        m_viewOffset = QPointF(width() / 2.0 - centerX * m_scale,
                               height() / 2.0 + centerY * m_scale);
        update();
        return;
    }

    // Calculate scale to fit
    double scaleX = width() * 0.8 / rangeX;
    double scaleY = height() * 0.8 / rangeY;
    m_scale = qMin(scaleX, scaleY);
    m_scale = qBound(m_minScale, m_scale, m_maxScale);

    // Center view
    m_viewOffset = QPointF(width() / 2.0 - centerX * m_scale,
                           height() / 2.0 + centerY * m_scale);
    update();
}

QPointF MapCanvas::worldToScreen(const Geometry::Point& worldPoint) const {
    // Qt screen coordinates: Y increases downward
    // World coordinates: Y increases upward
    return QPointF(worldPoint.x * m_scale + m_viewOffset.x(),
                   -worldPoint.y * m_scale + m_viewOffset.y());
}

Geometry::Point MapCanvas::screenToWorld(const QPointF& screenPoint) const {
    double worldX = (screenPoint.x() - m_viewOffset.x()) / m_scale;
    double worldY = -(screenPoint.y() - m_viewOffset.y()) / m_scale;
    return Geometry::Point(worldX, worldY);
}

void MapCanvas::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background with modern gradient
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, CANVAS_BG_TOP);
    gradient.setColorAt(1, CANVAS_BG_BOTTOM);
    painter.fillRect(rect(), gradient);

    // Draw grid
    if (m_showGrid) {
        drawGrid(painter);
    }

    // Draw origin
    drawOrigin(painter);

    // Draw map lines
    if (m_mapData) {
        drawLines(painter);
    }

    // Draw paths
    if (m_pathCollection) {
        drawPaths(painter);
    }

    // Draw reference points
    if (m_mapData) {
        drawReferencePoints(painter);
    }

    // Draw all robots
    if (m_showRobot) {
        for (int i = 0; i < m_robots.size(); ++i) {
            bool isPrimary = (i == m_primaryRobotIndex);
            drawRobot(painter, m_robots[i]);

            // Draw "PRIMARY" label on main robot
            if (isPrimary && m_robots.size() > 1) {
                QPointF robotPos = worldToScreen(m_robots[i].position);
                painter.setPen(ACCENT_ORANGE);
                painter.setFont(QFont("Arial", 9, QFont::Bold));
                painter.drawText(robotPos + QPointF(-20, -25), "PRIMARY");
            }
        }
    }

    // Draw current drawing
    if (m_isDrawing && m_currentTool == Tool::DrawLine) {
        painter.setPen(QPen(QColor(120, 180, 255), 2, Qt::DashLine));
        QPointF start = worldToScreen(m_drawStartPoint);
        QPointF end = worldToScreen(m_drawCurrentPoint);
        painter.drawLine(start, end);

        // Show dimension and angle while drawing
        Geometry::Line tempLine(m_drawStartPoint, m_drawCurrentPoint);
        drawDimension(painter, tempLine);
        drawAngle(painter, tempLine);
    }

    // Draw snap point indicators when DrawLine tool is active
    if (m_currentTool == Tool::DrawLine && m_snapToPoints) {
        QVector<Geometry::Point> snapPoints = getAllSnappablePoints();

        // Determine which point (if any) the cursor is near for highlighting
        Geometry::Point nearestSnap;
        double minDist = m_snapDistance;
        bool hasNearSnap = false;

        if (m_waitingForSecondClick) {
            for (const auto& snapPoint : snapPoints) {
                double dist = m_drawCurrentPoint.distanceTo(snapPoint);
                if (dist < minDist) {
                    minDist = dist;
                    nearestSnap = snapPoint;
                    hasNearSnap = true;
                }
            }
        }

        for (const auto& snapPoint : snapPoints) {
            QPointF screenPos = worldToScreen(snapPoint);

            // Check if point is visible on screen
            if (screenPos.x() < -50 || screenPos.x() > width() + 50 ||
                screenPos.y() < -50 || screenPos.y() > height() + 50) {
                continue;
            }

            // Check if this is the snap point we're near
            bool isNearCursor = hasNearSnap &&
                               std::abs(snapPoint.x - nearestSnap.x) < 0.001 &&
                               std::abs(snapPoint.y - nearestSnap.y) < 0.001;

            // Draw snap point indicator
            // Origin gets special treatment - larger and different color
            if (snapPoint.x == 0.0 && snapPoint.y == 0.0) {
                if (isNearCursor) {
                    painter.setPen(QPen(ACCENT_ORANGE, 3));
                    painter.setBrush(QColor(ACCENT_ORANGE.red(), ACCENT_ORANGE.green(), ACCENT_ORANGE.blue(), 200));
                    painter.drawEllipse(screenPos, 12, 12);
                } else {
                    painter.setPen(QPen(ACCENT_ORANGE, 2));
                    painter.setBrush(QColor(ACCENT_ORANGE.red(), ACCENT_ORANGE.green(), ACCENT_ORANGE.blue(), 120));
                    painter.drawEllipse(screenPos, 8, 8);
                }
            } else {
                // Line endpoints and other snap points
                if (isNearCursor) {
                    painter.setPen(QPen(ACCENT_TEAL, 3));
                    painter.setBrush(QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 170));
                    painter.drawEllipse(screenPos, 8, 8);
                } else {
                    QColor muted = QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 110);
                    painter.setPen(QPen(muted, 2));
                    painter.setBrush(QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 80));
                    painter.drawEllipse(screenPos, 5, 5);
                }
            }
        }
    }

    // Draw measurement
    if (m_measuring) {
        drawMeasurement(painter);
    }

    // Draw cursor coordinates (modern HUD style - always visible in top-left)
    // Use Menlo (macOS) or Monaco as fallback for monospace display
    QFont coordFont("Menlo", 11, QFont::Bold);
    coordFont.setStyleHint(QFont::Monospace);
    painter.setFont(coordFont);

    QString coordText = QString("📍 X: %1 m   Y: %2 m")
        .arg(m_cursorWorldPos.x, 0, 'f', 3)
        .arg(m_cursorWorldPos.y, 0, 'f', 3);

    QRectF coordRect = painter.fontMetrics().boundingRect(coordText);
    coordRect.adjust(-8, -4, 8, 4);
    coordRect.moveTo(12, 12);

    // Modern shadow effect
    painter.fillRect(coordRect.adjusted(2, 2, 2, 2), QColor(0, 0, 0, 120));

    painter.fillRect(coordRect, PANEL_BG);

    // Border
    painter.setPen(QPen(PANEL_BORDER, 1.5));
    painter.drawRoundedRect(coordRect, 4, 4);

    // Text
    painter.setPen(TEXT_PRIMARY);
    painter.drawText(coordRect, Qt::AlignCenter, coordText);

    if (m_currentTool == Tool::DrawLine) {
        QString snapText = QString("Snap %1 | %.2fm / %2°  (F to toggle)")
            .arg(m_fixedSnapEnabled ? "ON" : "OFF")
            .arg(m_fixedLengthStep, 0, 'f', 2)
            .arg(m_fixedAngleStep, 0, 'f', 0);

        QRectF snapRect = painter.fontMetrics().boundingRect(snapText);
        snapRect.adjust(-8, -4, 8, 4);
        snapRect.moveTopLeft(QPointF(12, coordRect.bottom() + 10));

        painter.fillRect(snapRect.adjusted(2, 2, 2, 2), QColor(0, 0, 0, 120));
        painter.fillRect(snapRect, PANEL_BG);
        painter.setPen(PANEL_BORDER);
        painter.drawRoundedRect(snapRect, 4, 4);
        painter.setPen(TEXT_PRIMARY);
        painter.drawText(snapRect, Qt::AlignCenter, snapText);
    }
}

void MapCanvas::drawGrid(QPainter& painter) {
    if (!m_mapData) return;

    double gridSize = m_mapData->gridSize; // meters
    double gridPixels = gridSize * m_scale;

    if (gridPixels < 10) return; // Don't draw if too small

    // Calculate visible range
    Geometry::Point topLeft = screenToWorld(QPointF(0, 0));
    Geometry::Point bottomRight = screenToWorld(QPointF(width(), height()));

    // Use double for intermediate calculations to prevent overflow
    double minGridXDouble = std::floor(qMin(topLeft.x, bottomRight.x) / gridSize);
    double maxGridXDouble = std::ceil(qMax(topLeft.x, bottomRight.x) / gridSize);
    double minGridYDouble = std::floor(qMin(topLeft.y, bottomRight.y) / gridSize);
    double maxGridYDouble = std::ceil(qMax(topLeft.y, bottomRight.y) / gridSize);

    // Prevent integer overflow and performance issues by clamping grid range
    const int MAX_GRID_LINES = 10000; // Maximum number of grid lines to draw
    const int MAX_INT_SAFE = 1000000; // Safe range for int calculations

    // Clamp to safe integer range to prevent overflow
    int minGridX = static_cast<int>(qBound(-static_cast<double>(MAX_INT_SAFE), minGridXDouble, static_cast<double>(MAX_INT_SAFE)));
    int maxGridX = static_cast<int>(qBound(-static_cast<double>(MAX_INT_SAFE), maxGridXDouble, static_cast<double>(MAX_INT_SAFE)));
    int minGridY = static_cast<int>(qBound(-static_cast<double>(MAX_INT_SAFE), minGridYDouble, static_cast<double>(MAX_INT_SAFE)));
    int maxGridY = static_cast<int>(qBound(-static_cast<double>(MAX_INT_SAFE), maxGridYDouble, static_cast<double>(MAX_INT_SAFE)));

    // Check if grid would have too many lines (performance protection)
    long long gridRangeX = static_cast<long long>(maxGridX) - static_cast<long long>(minGridX);
    long long gridRangeY = static_cast<long long>(maxGridY) - static_cast<long long>(minGridY);

    if (gridRangeX > MAX_GRID_LINES || gridRangeY > MAX_GRID_LINES) {
        // Too many grid lines - skip drawing to prevent freeze
        return;
    }

    // Draw vertical lines with major/minor distinction
    for (int i = minGridX; i <= maxGridX; ++i) {
        double x = i * gridSize;
        QPointF p1 = worldToScreen(Geometry::Point(x, minGridY * gridSize));
        QPointF p2 = worldToScreen(Geometry::Point(x, maxGridY * gridSize));

        // Major grid lines every 5 units (darker, thicker)
        if (i % 5 == 0 && i != 0) {
            painter.setPen(QPen(GRID_MAJOR_COLOR, 1.5));
        } else {
            painter.setPen(QPen(GRID_MINOR_COLOR, 1));
        }
        painter.drawLine(p1, p2);
    }

    // Draw horizontal lines with major/minor distinction
    for (int i = minGridY; i <= maxGridY; ++i) {
        double y = i * gridSize;
        QPointF p1 = worldToScreen(Geometry::Point(minGridX * gridSize, y));
        QPointF p2 = worldToScreen(Geometry::Point(maxGridX * gridSize, y));

        // Major grid lines every 5 units (darker, thicker)
        if (i % 5 == 0 && i != 0) {
            painter.setPen(QPen(GRID_MAJOR_COLOR, 1.5));
        } else {
            painter.setPen(QPen(GRID_MINOR_COLOR, 1));
        }
        painter.drawLine(p1, p2);
    }
}

void MapCanvas::drawOrigin(QPainter& painter) {
    QPointF origin = worldToScreen(Geometry::Point(0, 0));

    // Draw axis lines extending across visible area (global coordinate system)
    painter.setPen(QPen(QColor(ACCENT_ORANGE.red(), ACCENT_ORANGE.green(), ACCENT_ORANGE.blue(), 140), 1, Qt::DashLine));
    painter.drawLine(QPointF(0, origin.y()), QPointF(width(), origin.y()));  // X-axis
    painter.drawLine(QPointF(origin.x(), 0), QPointF(origin.x(), height()));  // Y-axis

    // Draw larger crosshair at origin
    painter.setPen(QPen(ACCENT_ORANGE, 3));
    painter.drawLine(origin + QPointF(-20, 0), origin + QPointF(20, 0));
    painter.drawLine(origin + QPointF(0, -20), origin + QPointF(0, 20));

    // Draw origin circle
    painter.setBrush(QBrush(ACCENT_ORANGE));
    painter.drawEllipse(origin, 8, 8);

    // Draw axis labels
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.setPen(TEXT_PRIMARY);

    // X-axis label (pointing right)
    painter.fillRect(QRectF(origin.x() + 25, origin.y() - 12, 35, 24), PANEL_BG);
    painter.drawText(QRectF(origin.x() + 25, origin.y() - 12, 35, 24), Qt::AlignCenter, "+X");

    // Y-axis label (pointing up)
    painter.fillRect(QRectF(origin.x() - 12, origin.y() - 35, 24, 24), PANEL_BG);
    painter.drawText(QRectF(origin.x() - 12, origin.y() - 35, 24, 24), Qt::AlignCenter, "+Y");

    // Origin label
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.fillRect(QRectF(origin.x() + 12, origin.y() + 12, 60, 20), PANEL_BG);
    painter.drawText(QRectF(origin.x() + 12, origin.y() + 12, 60, 20), Qt::AlignCenter, "Origin (0,0)");
}

void MapCanvas::drawLines(QPainter& painter) {
    for (int i = 0; i < m_mapData->lines.size(); ++i) {
        const auto& line = m_mapData->lines[i];

        // Highlight selected line
        if (i == m_selectedLineIndex) {
            painter.setPen(QPen(ACCENT_TEAL, 5));  // Thicker highlighted line for selection
        } else {
            painter.setPen(QPen(Qt::black, 3));
        }

        QPointF start = worldToScreen(line.start);
        QPointF end = worldToScreen(line.end);
        painter.drawLine(start, end);

        if (m_showDimensions) {
            drawDimension(painter, line);
        }
    }
}

void MapCanvas::drawDimension(QPainter& painter, const Geometry::Line& line) {
    QPointF start = worldToScreen(line.start);
    QPointF end = worldToScreen(line.end);
    QPointF mid = (start + end) / 2.0;

    // Calculate perpendicular offset for text (SolidWorks style)
    QPointF vec = end - start;
    double len = std::sqrt(vec.x() * vec.x() + vec.y() * vec.y());
    if (len < 1e-6) return;

    QPointF perp(-vec.y() / len, vec.x() / len);

    // Position above the line (perpendicular offset)
    // Make offset larger to avoid overlap with angle
    QPointF textPos = mid + perp * 25;

    // Draw dimension text with nice formatting
    QString dimText = QString::number(line.length(), 'f', 3) + " m";
    painter.setPen(TEXT_PRIMARY);
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    QRectF textRect = painter.fontMetrics().boundingRect(dimText);
    textRect.moveCenter(textPos);

    painter.fillRect(textRect.adjusted(-3, -2, 3, 2), PANEL_BG);
    painter.setPen(TEXT_PRIMARY);
    painter.drawText(textRect, Qt::AlignCenter, dimText);
}

void MapCanvas::drawPaths(QPainter& painter) {
    for (int pathIdx = 0; pathIdx < m_pathCollection->paths.size(); ++pathIdx) {
        const auto& path = m_pathCollection->paths[pathIdx];
        if (!path.visible || path.waypoints.isEmpty()) {
            continue;
        }

        // Draw path line
        painter.setPen(QPen(path.color, 2, Qt::DashLine));
        for (int i = 1; i < path.waypoints.size(); ++i) {
            QPointF p1 = worldToScreen(path.waypoints[i - 1].position);
            QPointF p2 = worldToScreen(path.waypoints[i].position);
            painter.drawLine(p1, p2);
        }

        // Draw waypoints
        for (int i = 0; i < path.waypoints.size(); ++i) {
            const auto& wp = path.waypoints[i];
            QPointF p = worldToScreen(wp.position);

            // Highlight selected waypoint
            bool isSelected = (pathIdx == m_selectedPathIndex && i == m_selectedWaypointIndex);
            if (isSelected) {
                painter.setPen(QPen(ACCENT_ORANGE, 3));
                painter.setBrush(QBrush(ACCENT_ORANGE));
                painter.drawEllipse(p, 9, 9);  // Larger circle for selection
            } else {
                painter.setPen(QPen(path.color, 2));
                painter.setBrush(QBrush(path.color.lighter(150)));
                painter.drawEllipse(p, 6, 6);
            }

            // Draw heading arrow (longer for visibility)
            double arrowLen = 25;
            QPointF arrowEnd = p + QPointF(arrowLen * std::cos(wp.heading),
                                           -arrowLen * std::sin(wp.heading));
            painter.setPen(QPen(path.color, 3));
            painter.drawLine(p, arrowEnd);

            // Draw arrowhead
            double arrowHeadLen = 8;
            double arrowAngle = 0.4; // radians
            QPointF arrowTip1 = arrowEnd - QPointF(
                arrowHeadLen * std::cos(wp.heading - arrowAngle),
                -arrowHeadLen * std::sin(wp.heading - arrowAngle));
            QPointF arrowTip2 = arrowEnd - QPointF(
                arrowHeadLen * std::cos(wp.heading + arrowAngle),
                -arrowHeadLen * std::sin(wp.heading + arrowAngle));
            painter.drawLine(arrowEnd, arrowTip1);
            painter.drawLine(arrowEnd, arrowTip2);

            // Draw heading in degrees
            double headingDeg = wp.heading * 180.0 / M_PI;
            while (headingDeg > 180) headingDeg -= 360;
            while (headingDeg < -180) headingDeg += 360;

            QString headingText = QString("%1°").arg(headingDeg, 0, 'f', 0);
            painter.setPen(TEXT_PRIMARY);
            painter.setFont(QFont("Arial", 9, QFont::Bold));

            QRectF textRect = painter.fontMetrics().boundingRect(headingText);
            textRect.moveCenter(p + QPointF(0, 20));

            painter.fillRect(textRect.adjusted(-2, -1, 2, 1), PANEL_BG);
            painter.setPen(TEXT_PRIMARY);
            painter.drawText(textRect, Qt::AlignCenter, headingText);
        }
    }
}

void MapCanvas::drawRobot(QPainter& painter, const Geometry::RobotPose& pose) {
    QPointF center = worldToScreen(pose.position);

    painter.setPen(QPen(ACCENT_TEAL, 2));
    painter.setBrush(QBrush(QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 60)));

    switch (pose.shape) {
        case Geometry::RobotShape::Rectangle:
            drawRobotRectangle(painter, center, pose.width, pose.length, pose.heading);
            break;
        case Geometry::RobotShape::Square:
            drawRobotSquare(painter, center, pose.width, pose.heading);
            break;
        case Geometry::RobotShape::Triangle:
            drawRobotTriangle(painter, center, pose.width, pose.heading);
            break;
    }

    // Draw heading indicator
    painter.setPen(QPen(ACCENT_ORANGE, 3));
    double indicatorLen = pose.length * m_scale * 0.6;
    QPointF headingEnd = center + QPointF(indicatorLen * std::cos(pose.heading),
                                          -indicatorLen * std::sin(pose.heading));
    painter.drawLine(center, headingEnd);
}

void MapCanvas::drawRobotRectangle(QPainter& painter, const QPointF& center,
                                    double width, double length, double heading) {
    double w = width * m_scale;
    double l = length * m_scale;

    QRectF rect(-l / 2, -w / 2, l, w);
    painter.save();
    painter.translate(center);
    painter.rotate(-heading * 180.0 / M_PI);
    painter.drawRect(rect);
    painter.restore();
}

void MapCanvas::drawRobotSquare(QPainter& painter, const QPointF& center,
                                 double size, double heading) {
    drawRobotRectangle(painter, center, size, size, heading);
}

void MapCanvas::drawRobotTriangle(QPainter& painter, const QPointF& center,
                                   double size, double heading) {
    double s = size * m_scale;

    QPainterPath path;
    path.moveTo(s / 2, 0);              // Front point
    path.lineTo(-s / 2, s / 2);         // Back left
    path.lineTo(-s / 2, -s / 2);        // Back right
    path.closeSubpath();

    painter.save();
    painter.translate(center);
    painter.rotate(-heading * 180.0 / M_PI);
    painter.drawPath(path);
    painter.restore();
}

void MapCanvas::drawMeasurement(QPainter& painter) {
    QPointF start = worldToScreen(m_measureStart);
    QPointF end = worldToScreen(m_measureEnd);

    // Draw measurement line
    painter.setPen(QPen(ACCENT_TEAL, 2, Qt::DashLine));
    painter.drawLine(start, end);

    // Draw snap indicators
    if (m_measureSnappedToRobot) {
        // Highlight snapped robot with green circle
        painter.setPen(QPen(ACCENT_TEAL.lighter(150), 3));
        painter.setBrush(QBrush(QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 70)));
        painter.drawEllipse(end, 15, 15);

        // Draw "ROBOT" label
        painter.setPen(TEXT_PRIMARY);
        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(end + QPointF(-15, -20), "ROBOT");
    } else if (m_measureSnappedToLine) {
        // Highlight snap point on line with yellow circle
        painter.setPen(QPen(ACCENT_ORANGE, 3));
        painter.setBrush(QBrush(QColor(ACCENT_ORANGE.red(), ACCENT_ORANGE.green(), ACCENT_ORANGE.blue(), 90)));
        painter.drawEllipse(end, 12, 12);

        // Draw "LINE" label
        painter.setPen(TEXT_PRIMARY);
        painter.setFont(QFont("Arial", 9, QFont::Bold));
        painter.drawText(end + QPointF(-12, -18), "LINE");
    }

    // Draw distance text
    double distance = m_measureStart.distanceTo(m_measureEnd);
    QString distText = QString("Distance: %1 m").arg(distance, 0, 'f', 3);

    // Add snap info to text
    if (m_measureSnappedToRobot) {
        distText += QString(" [to Robot #%1]").arg(m_measureSnappedRobotIndex + 1);
    } else if (m_measureSnappedToLine) {
        distText += QString(" [to Line #%1]").arg(m_measureSnappedLineIndex + 1);
    }

    QPointF mid = (start + end) / 2.0;
    painter.setPen(TEXT_PRIMARY);
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    QRectF textRect = painter.fontMetrics().boundingRect(distText);
    textRect.moveCenter(mid);

    painter.fillRect(textRect.adjusted(-4, -4, 4, 4), PANEL_BG);
    painter.setPen(TEXT_PRIMARY);
    painter.drawText(textRect, Qt::AlignCenter, distText);

    if (m_measureLocked) {
        painter.setPen(TEXT_PRIMARY);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(end + QPointF(15, 15), "[Locked]");
    }
}

void MapCanvas::mousePressEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    Geometry::Point worldPos = screenToWorld(pos);

    if (event->button() == Qt::LeftButton) {
        // Check for robot dragging or waypoint editing first (for Select tool or no tool)
        if (m_currentTool == Tool::Select || m_currentTool == Tool::None) {
            // Check if clicking on waypoint heading handle (takes priority)
            int pathIdx = -1, wpIdx = -1;
            if (findNearestWaypoint(worldPos, pathIdx, wpIdx)) {
                if (isNearWaypointHeadingHandle(worldPos, pathIdx, wpIdx)) {
                    m_isEditingHeading = true;
                    m_editingPathIndex = pathIdx;
                    m_editingWaypointIndex = wpIdx;
                    // Also select the waypoint
                    m_selectedPathIndex = pathIdx;
                    m_selectedWaypointIndex = wpIdx;
                    m_selectedLineIndex = -1;
                    setCursor(Qt::CrossCursor);
                    update();
                    return;
                }

                // Not on heading handle, but near waypoint body - start dragging waypoint
                m_isDraggingWaypoint = true;
                m_draggedPathIndex = pathIdx;
                m_draggedWaypointIndex = wpIdx;
                // Select the waypoint
                m_selectedPathIndex = pathIdx;
                m_selectedWaypointIndex = wpIdx;
                m_selectedLineIndex = -1;
                setCursor(Qt::ClosedHandCursor);
                emit statusMessage(QString("Waypoint selected - Press Delete to remove"));
                update();
                return;
            }

            // Check if clicking on a line
            if (m_mapData) {
                int lineIdx = findNearestLine(worldPos, LINE_PROXIMITY_THRESHOLD);
                if (lineIdx >= 0) {
                    m_selectedLineIndex = lineIdx;
                    m_selectedPathIndex = -1;
                    m_selectedWaypointIndex = -1;
                    emit statusMessage(QString("Line selected - Press Delete to remove or double-click to edit"));
                    update();
                    return;
                }
            }

            // Check if clicking on robot
            int robotIdx = findNearestRobot(worldPos);
            if (robotIdx >= 0) {
                m_isDraggingRobot = true;
                m_draggedRobotIndex = robotIdx;
                // Clear line/waypoint selection
                m_selectedLineIndex = -1;
                m_selectedPathIndex = -1;
                m_selectedWaypointIndex = -1;
                setCursor(Qt::ClosedHandCursor);
                update();
                return;
            }

            // Clicked on empty space - clear selection
            clearSelection();
            update();
        }

        // Handle tools
        switch (m_currentTool) {
            case Tool::DrawLine:
                handleDrawLinePress(pos);
                break;
            case Tool::DrawPath:
                handleDrawPathPress(pos);
                break;
            case Tool::Measure:
                handleMeasurePress(pos);
                break;
            case Tool::Pan:
                m_isPanning = true;
                m_lastMousePos = pos;
                setCursor(Qt::ClosedHandCursor);
                break;
            case Tool::AddReference:
                handleAddReferencePress(pos);
                break;
            default:
                break;
        }
    }
    else if (event->button() == Qt::RightButton) {
        // Cancel current operation
        m_isDrawing = false;
        m_waitingForSecondClick = false;
        m_measuring = false;
        m_isDraggingRobot = false;
        m_isDraggingWaypoint = false;
        m_isEditingHeading = false;
        update();
    }
}

void MapCanvas::mouseMoveEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    Geometry::Point worldPos = screenToWorld(pos);

    // Update cursor position for display
    m_cursorWorldPos = worldPos;
    update(); // Trigger repaint to show cursor coordinates

    // Handle robot dragging or rotating
    if (m_isDraggingRobot && m_draggedRobotIndex >= 0 && m_draggedRobotIndex < m_robots.size()) {
        // Check if Shift is held for rotation
        if (event->modifiers() & Qt::ShiftModifier) {
            // Rotate robot: calculate angle from robot center to mouse
            auto& robot = m_robots[m_draggedRobotIndex];
            double dx = worldPos.x - robot.position.x;
            double dy = worldPos.y - robot.position.y;
            robot.heading = std::atan2(dy, dx);

            double headingDeg = robot.heading * 180.0 / M_PI;
            update();
            emit statusMessage(QString("Robot %1 heading: %2°")
                .arg(m_draggedRobotIndex + 1)
                .arg(headingDeg, 0, 'f', 1));
        } else {
            // Move robot position
            m_robots[m_draggedRobotIndex].position = worldPos;
            update();
            emit statusMessage(QString("Robot %1 at X: %2 m, Y: %3 m (Hold Shift to rotate)")
                .arg(m_draggedRobotIndex + 1)
                .arg(worldPos.x, 0, 'f', 3)
                .arg(worldPos.y, 0, 'f', 3));
        }
        return;
    }

    // Handle waypoint dragging or rotating
    if (m_isDraggingWaypoint && m_pathCollection &&
        m_draggedPathIndex >= 0 && m_draggedPathIndex < m_pathCollection->paths.size()) {
        auto& path = m_pathCollection->paths[m_draggedPathIndex];
        if (m_draggedWaypointIndex >= 0 && m_draggedWaypointIndex < path.waypoints.size()) {
            auto& wp = path.waypoints[m_draggedWaypointIndex];

            // Check if Shift is held for rotation
            if (event->modifiers() & Qt::ShiftModifier) {
                // Rotate waypoint heading: calculate angle from waypoint to mouse
                double dx = worldPos.x - wp.position.x;
                double dy = worldPos.y - wp.position.y;
                wp.heading = std::atan2(dy, dx);

                double headingDeg = wp.heading * 180.0 / M_PI;
                update();
                emit statusMessage(QString("Waypoint heading: %1° (Path %2, WP %3)")
                    .arg(headingDeg, 0, 'f', 1)
                    .arg(m_draggedPathIndex + 1)
                    .arg(m_draggedWaypointIndex + 1));
            } else {
                // Move waypoint position
                wp.position = worldPos;
                update();
                emit statusMessage(QString("Waypoint at X: %1 m, Y: %2 m (Hold Shift to rotate)")
                    .arg(worldPos.x, 0, 'f', 3)
                    .arg(worldPos.y, 0, 'f', 3));
            }
            return;
        }
    }

    // Handle waypoint heading editing
    if (m_isEditingHeading && m_pathCollection &&
        m_editingPathIndex >= 0 && m_editingPathIndex < m_pathCollection->paths.size()) {
        auto& path = m_pathCollection->paths[m_editingPathIndex];
        if (m_editingWaypointIndex >= 0 && m_editingWaypointIndex < path.waypoints.size()) {
            auto& wp = path.waypoints[m_editingWaypointIndex];

            // Calculate angle from waypoint to mouse
            double dx = worldPos.x - wp.position.x;
            double dy = worldPos.y - wp.position.y;
            wp.heading = std::atan2(dy, dx);

            update();

            double headingDeg = wp.heading * 180.0 / M_PI;
            emit statusMessage(QString("Heading: %1°").arg(headingDeg, 0, 'f', 1));
            return;
        }
    }

    // Handle panning
    if (m_isPanning) {
        QPointF delta = pos - m_lastMousePos;
        m_viewOffset += delta;
        m_lastMousePos = pos;
        update();
    }
    else if (m_isDrawing && m_currentTool == Tool::DrawLine) {
        handleDrawLineMove(pos);
    }
    else if (m_measuring) {
        handleMeasureMove(pos);
    }

    m_cursorWorldPos = worldPos;
    emit cursorPositionChanged(worldPos.x, worldPos.y);
}

void MapCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Stop robot dragging
        if (m_isDraggingRobot) {
            m_isDraggingRobot = false;
            m_draggedRobotIndex = -1;
            setCursor(Qt::ArrowCursor);
        }
        // Stop waypoint dragging
        else if (m_isDraggingWaypoint) {
            m_isDraggingWaypoint = false;
            m_draggedPathIndex = -1;
            m_draggedWaypointIndex = -1;
            setCursor(Qt::ArrowCursor);
        }
        // Stop heading editing
        else if (m_isEditingHeading) {
            m_isEditingHeading = false;
            m_editingPathIndex = -1;
            m_editingWaypointIndex = -1;
            setCursor(Qt::ArrowCursor);
        }
        // Stop panning
        else if (m_isPanning) {
            m_isPanning = false;
            setCursor(Qt::OpenHandCursor);
        }
        // Finish drawing line
        else if (m_isDrawing && m_currentTool == Tool::DrawLine) {
            handleDrawLineRelease(event->position());
        }
    }
}

void MapCanvas::wheelEvent(QWheelEvent* event) {
    // Zoom at mouse position
    QPointF mousePos = event->position();
    Geometry::Point worldPosBeforeZoom = screenToWorld(mousePos);

    if (event->angleDelta().y() > 0) {
        m_scale = qMin(m_scale * 1.1, m_maxScale);
    } else {
        m_scale = qMax(m_scale / 1.1, m_minScale);
    }

    // Adjust offset to keep mouse position fixed
    QPointF worldPosAfterZoom = worldToScreen(worldPosBeforeZoom);
    m_viewOffset += (mousePos - worldPosAfterZoom);

    update();
}

void MapCanvas::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    update();
}

void MapCanvas::handleDrawLinePress(const QPointF& pos) {
    Geometry::Point worldPos = screenToWorld(pos);

    // Snap if enabled
    if (m_snapToPoints) {
        worldPos = snapToNearestPoint(worldPos);
    }

    if (!m_waitingForSecondClick) {
        // FIRST CLICK: Set start point
        m_drawStartPoint = worldPos;
        m_drawCurrentPoint = worldPos;
        m_lastDrawPoint = worldPos;
        m_waitingForSecondClick = true;
        m_isDrawing = true;

        emit statusMessage(QString("Start point set at X: %1 m, Y: %2 m - Click to set end point, or ESC to cancel")
            .arg(worldPos.x, 0, 'f', 3)
            .arg(worldPos.y, 0, 'f', 3));

        update();
    } else {
        // SECOND CLICK: Set end point and create line
        // Use constrained endpoint if in constrained mode, otherwise use mouse position
        if (!m_constrainedDrawing) {
            m_drawCurrentPoint = worldPos;
            if (m_fixedSnapEnabled) {
                m_drawCurrentPoint = applyFixedLengthAngleSnap(m_drawStartPoint, m_drawCurrentPoint);
            }
        }
        // If constrained, m_drawCurrentPoint is already set by applyConstrainedDimensions()

        Geometry::Line line(m_drawStartPoint, m_drawCurrentPoint);

        // Only add if line has some length
        if (line.length() > 0.01) {
            if (m_mapData) {
                m_mapData->addLine(line);
                emit lineAdded(line);

                emit statusMessage(QString("Line added (length: %1 m) - Click to continue, D for dimensions, or ESC to finish")
                    .arg(line.length(), 0, 'f', 3));
            }

            // Continue chain: start next line from current end point
            m_drawStartPoint = m_drawCurrentPoint;
            m_lastDrawPoint = m_drawCurrentPoint;

            // Reset constrained drawing for next line
            m_constrainedDrawing = false;

            // Keep m_waitingForSecondClick = true to continue drawing
        } else {
            // Line too short, cancel
            m_waitingForSecondClick = false;
            m_isDrawing = false;
            m_constrainedDrawing = false;
            emit statusMessage(QString("Line too short - cancelled"));
        }

        update();
    }
}

void MapCanvas::handleDrawLineMove(const QPointF& pos) {
    // Only update preview when waiting for second click (SolidWorks-style)
    if (!m_waitingForSecondClick) {
        return;
    }

    // If in constrained drawing mode, line is locked to exact dimensions
    if (m_constrainedDrawing) {
        // Just update display - endpoint is already calculated
        update();
        return;
    }

    Geometry::Point newPoint = screenToWorld(pos);

    // Snap end point if enabled
    if (m_snapToPoints) {
        newPoint = snapToNearestPoint(newPoint);
    }

    // STABILIZATION: Only update if moved beyond minimum distance threshold
    double minMoveDist = getMinimumMoveDistance();
    double distFromLast = newPoint.distanceTo(m_lastDrawPoint);

    if (distFromLast < minMoveDist) {
        // Too close to last update - ignore to reduce jitter
        return;
    }

    m_drawCurrentPoint = newPoint;
    if (m_fixedSnapEnabled) {
        m_drawCurrentPoint = applyFixedLengthAngleSnap(m_drawStartPoint, m_drawCurrentPoint);
    }
    m_lastDrawPoint = m_drawCurrentPoint;

    // Show live distance and angle
    double length = m_drawStartPoint.distanceTo(m_drawCurrentPoint);
    double angle = std::atan2(m_drawCurrentPoint.y - m_drawStartPoint.y,
                              m_drawCurrentPoint.x - m_drawStartPoint.x) * 180.0 / M_PI;

    QString snapHint;
    if (m_fixedSnapEnabled) {
        snapHint = QString(" | Snap %.2fm / %3° (F toggles)")
            .arg(m_fixedLengthStep, 0, 'f', 2)
            .arg(m_fixedAngleStep, 0, 'f', 0);
    }

    emit statusMessage(QString("Length: %1 m, Angle: %2° - Click to place, or press D for exact dimensions%3")
        .arg(length, 0, 'f', 3)
        .arg(angle, 0, 'f', 1)
        .arg(snapHint));

    update();
}

void MapCanvas::handleDrawLineRelease(const QPointF& pos) {
    // SolidWorks-style: We don't use mouse release - everything is click-based
    // This function is kept for compatibility but does nothing
    Q_UNUSED(pos);
}

void MapCanvas::handleDrawPathPress(const QPointF& pos) {
    Geometry::Point worldPos = screenToWorld(pos);
    Geometry::Waypoint wp(worldPos, 0.0, 1.0);

    if (m_pathCollection) {
        PathData* activePath = m_pathCollection->getActivePath();
        if (activePath) {
            activePath->addWaypoint(wp);
            emit waypointAdded(wp);
            update();
        }
    }
}

void MapCanvas::handleMeasurePress(const QPointF& pos) {
    if (!m_measuring || m_measureLocked) {
        m_measuring = true;
        m_measureLocked = false;
        m_measureStart = screenToWorld(pos);

        bool snappedStart = false;
        m_measureStart = snapToNearestLineOrRobot(m_measureStart, snappedStart);

        m_measureEnd = m_measureStart;
        m_measureSnappedToLine = false;
        m_measureSnappedToRobot = false;
        m_measureSnappedLineIndex = -1;
        m_measureSnappedRobotIndex = -1;
        emit statusMessage("Measurement started - click again to finish, or press Delete/Esc to clear");
        update();
    } else {
        m_measureLocked = true;
        emit statusMessage("Measurement locked. Press Delete or Esc to clear.");
    }
}

void MapCanvas::handleMeasureMove(const QPointF& pos) {
    if (m_measureLocked) {
        return;
    }
    m_measureEnd = screenToWorld(pos);

    // Check if near line or robot - snap to closest point on line or robot center
    bool snapped = false;
    m_measureEnd = snapToNearestLineOrRobot(m_measureEnd, snapped);

    // Emit distance measurement
    double distance = m_measureStart.distanceTo(m_measureEnd);
    emit distanceMeasured(distance);
    update();
}

// Snap point to nearest line endpoint or reference point
Geometry::Point MapCanvas::snapToNearestPoint(const Geometry::Point& point) {
    if (!m_snapToPoints || !m_mapData) {
        return point;
    }

    QVector<Geometry::Point> snappablePoints = getAllSnappablePoints();
    
    Geometry::Point closestPoint = point;
    double minDistance = m_snapDistance;

    for (const auto& snapPoint : snappablePoints) {
        double dist = point.distanceTo(snapPoint);
        if (dist < minDistance) {
            minDistance = dist;
            closestPoint = snapPoint;
        }
    }

    return closestPoint;
}

// Get all points that can be snapped to
QVector<Geometry::Point> MapCanvas::getAllSnappablePoints() const {
    QVector<Geometry::Point> points;

    // ALWAYS add origin (0,0) as first snap point
    points.append(Geometry::Point(0, 0));

    if (!m_mapData) {
        return points;
    }

    // Add all line endpoints
    for (const auto& line : m_mapData->lines) {
        points.append(line.start);
        points.append(line.end);
    }

    // Add all reference points
    for (const auto& refPoint : m_mapData->referencePoints) {
        points.append(refPoint.position);
    }

    // Add path waypoints
    if (m_pathCollection) {
        for (const auto& path : m_pathCollection->paths) {
            for (const auto& wp : path.waypoints) {
                points.append(wp.position);
            }
        }
    }

    return points;
}

// Draw reference points
void MapCanvas::drawReferencePoints(QPainter& painter) {
    if (!m_mapData) return;

    for (const auto& refPoint : m_mapData->referencePoints) {
        QPointF pos = worldToScreen(refPoint.position);

        // Draw diamond shape
        painter.setPen(QPen(ACCENT_TEAL, 2));
        painter.setBrush(QBrush(QColor(ACCENT_TEAL.red(), ACCENT_TEAL.green(), ACCENT_TEAL.blue(), 80)));

        QPainterPath diamond;
        double size = 8;
        diamond.moveTo(pos.x(), pos.y() - size);
        diamond.lineTo(pos.x() + size, pos.y());
        diamond.lineTo(pos.x(), pos.y() + size);
        diamond.lineTo(pos.x() - size, pos.y());
        diamond.closeSubpath();
        painter.drawPath(diamond);

        // Draw name
        painter.setPen(TEXT_PRIMARY);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        QRectF textRect = painter.fontMetrics().boundingRect(refPoint.name);
        textRect.moveCenter(pos + QPointF(0, -15));
        painter.fillRect(textRect.adjusted(-2, -2, 2, 2), PANEL_BG);
        painter.drawText(textRect, Qt::AlignCenter, refPoint.name);

        // Draw heading arrow if it has one
        if (refPoint.hasHeading) {
            double arrowLen = 20;
            QPointF arrowEnd = pos + QPointF(arrowLen * std::cos(refPoint.heading),
                                             -arrowLen * std::sin(refPoint.heading));
            painter.setPen(QPen(ACCENT_TEAL, 3));
            painter.drawLine(pos, arrowEnd);
        }
    }
}

// Draw angle label for a line (SolidWorks style: near start point)
void MapCanvas::drawAngle(QPainter& painter, const Geometry::Line& line) {
    QPointF start = worldToScreen(line.start);
    QPointF end = worldToScreen(line.end);

    // Calculate line direction vector
    QPointF vec = end - start;
    double len = std::sqrt(vec.x() * vec.x() + vec.y() * vec.y());
    if (len < 1e-6) return;

    double angleDeg = line.angleDegrees();

    // Normalize angle to -180 to 180
    while (angleDeg > 180) angleDeg -= 360;
    while (angleDeg < -180) angleDeg += 360;

    QString angleText = QString("%1°").arg(angleDeg, 0, 'f', 1);

    // Smart positioning based on line orientation to avoid overlap
    QPointF perp(-vec.y() / len, vec.x() / len);
    QPointF anglePos;

    // Check if line is more horizontal or vertical
    double absAngle = std::abs(angleDeg);
    bool isNearHorizontal = (absAngle < 30 || absAngle > 150);

    if (isNearHorizontal) {
        // For horizontal lines: place angle BELOW the line (opposite side from distance)
        anglePos = start + (vec / len) * 50 + perp * 25; // 50px along, 25px below
    } else {
        // For vertical/diagonal lines: place angle near start, offset perpendicular
        anglePos = start + (vec / len) * 40 - perp * 20;
    }

    painter.setPen(TEXT_PRIMARY);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    QRectF textRect = painter.fontMetrics().boundingRect(angleText);
    textRect.moveCenter(anglePos);

    painter.fillRect(textRect.adjusted(-3, -2, 3, 2), PANEL_BG);
    painter.setPen(TEXT_PRIMARY);
    painter.drawText(textRect, Qt::AlignCenter, angleText);
}

// Handle adding reference point
void MapCanvas::handleAddReferencePress(const QPointF& pos) {
    Geometry::Point worldPos = screenToWorld(pos);

    // Snap to nearest point if enabled
    if (m_snapToPoints) {
        worldPos = snapToNearestPoint(worldPos);
    }

    // Create reference point with default name
    int refNum = m_mapData ? m_mapData->referencePoints.size() + 1 : 1;
    QString name = QString("Ref %1").arg(refNum);

    Geometry::ReferencePoint refPoint(worldPos, name, 0.0, false);

    if (m_mapData) {
        m_mapData->addReferencePoint(refPoint);
        emit referencePointAdded(refPoint);
        update();
    }
}

// Handle double-click on waypoint to edit
void MapCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    Geometry::Point worldPos = screenToWorld(pos);

    if (m_pathCollection) {
        for (int pathIdx = 0; pathIdx < m_pathCollection->paths.size(); ++pathIdx) {
            const auto& path = m_pathCollection->paths[pathIdx];
            for (int wpIdx = 0; wpIdx < path.waypoints.size(); ++wpIdx) {
                const auto& wp = path.waypoints[wpIdx];
                double dist = worldPos.distanceTo(wp.position);
                if (dist < WAYPOINT_CLICK_THRESHOLD) {
                    emit waypointDoubleClicked(pathIdx, wpIdx);
                    return;
                }
            }
        }
    }

    if (m_mapData) {
        int lineIdx = findNearestLine(worldPos, LINE_PROXIMITY_THRESHOLD);
        if (lineIdx >= 0) {
            emit lineDoubleClicked(lineIdx);
            return;
        }
    }
}

// Find nearest robot to a point
int MapCanvas::findNearestRobot(const Geometry::Point& point, double maxDistance) {
    int nearestIdx = -1;
    double minDist = maxDistance;

    for (int i = 0; i < m_robots.size(); ++i) {
        double dist = point.distanceTo(m_robots[i].position);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }

    return nearestIdx;
}

// Find nearest waypoint to a point
bool MapCanvas::findNearestWaypoint(const Geometry::Point& point, int& pathIdx, int& wpIdx, double maxDistance) {
    if (!m_pathCollection) return false;

    double minDist = maxDistance;
    bool found = false;

    for (int pi = 0; pi < m_pathCollection->paths.size(); ++pi) {
        const auto& path = m_pathCollection->paths[pi];
        if (!path.visible) continue;

        for (int wi = 0; wi < path.waypoints.size(); ++wi) {
            double dist = point.distanceTo(path.waypoints[wi].position);
            if (dist < minDist) {
                minDist = dist;
                pathIdx = pi;
                wpIdx = wi;
                found = true;
            }
        }
    }

    return found;
}

// Check if near waypoint heading handle (the arrow end)
bool MapCanvas::isNearWaypointHeadingHandle(const Geometry::Point& point, int pathIdx, int wpIdx) {
    if (!m_pathCollection) return false;
    if (pathIdx < 0 || pathIdx >= m_pathCollection->paths.size()) return false;

    const auto& path = m_pathCollection->paths[pathIdx];
    if (wpIdx < 0 || wpIdx >= path.waypoints.size()) return false;

    const auto& wp = path.waypoints[wpIdx];

    // Calculate where the arrow end is (25 pixels = arrowLen from drawPaths)
    double arrowLen = 25.0 / m_scale; // Convert screen pixels to world meters
    Geometry::Point arrowEnd(
        wp.position.x + arrowLen * std::cos(wp.heading),
        wp.position.y + arrowLen * std::sin(wp.heading)
    );

    double dist = point.distanceTo(arrowEnd);
    return dist < WAYPOINT_HANDLE_THRESHOLD;
}

// Find line near a point (for selection)
int MapCanvas::findNearestLine(const Geometry::Point& point, double maxDistance) {
    if (!m_mapData) return -1;

    int nearestIdx = -1;
    double minDist = maxDistance;

    for (int i = 0; i < m_mapData->lines.size(); ++i) {
        double dist = m_mapData->lines[i].distanceToPoint(point);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }

    return nearestIdx;
}

// Get closest point on a line segment to a given point
Geometry::Point MapCanvas::getClosestPointOnLine(const Geometry::Line& line, const Geometry::Point& point) const {
    double dx = line.end.x - line.start.x;
    double dy = line.end.y - line.start.y;

    double lineLength = line.length();
    if (lineLength < 1e-6) {
        return line.start;
    }

    // Calculate parameter t for projection
    double t = ((point.x - line.start.x) * dx + (point.y - line.start.y) * dy) / (lineLength * lineLength);

    // Clamp to [0, 1] to keep on line segment
    t = std::max(0.0, std::min(1.0, t));

    // Return closest point on line
    return Geometry::Point(
        line.start.x + t * dx,
        line.start.y + t * dy
    );
}

// Snap to nearest line or robot for measuring tool
Geometry::Point MapCanvas::snapToNearestLineOrRobot(const Geometry::Point& point, bool& snapped) {
    snapped = false;
    Geometry::Point result = point;
    double minDist = 0.1; // 10cm max snap distance (less sensitive, SolidWorks style)

    // Reset snap flags
    m_measureSnappedToLine = false;
    m_measureSnappedToRobot = false;
    m_measureSnappedLineIndex = -1;
    m_measureSnappedRobotIndex = -1;

    // Check robots first (higher priority)
    for (int i = 0; i < m_robots.size(); ++i) {
        double dist = point.distanceTo(m_robots[i].position);
        if (dist < minDist) {
            minDist = dist;
            result = m_robots[i].position;
            snapped = true;
            m_measureSnappedToRobot = true;
            m_measureSnappedRobotIndex = i;
            m_measureSnappedToLine = false;
        }
    }

    // Check lines (snap to closest point on line)
    if (m_mapData) {
        for (int i = 0; i < m_mapData->lines.size(); ++i) {
            const auto& line = m_mapData->lines[i];
            Geometry::Point closestPoint = getClosestPointOnLine(line, point);
            double dist = point.distanceTo(closestPoint);

            if (dist < minDist) {
                minDist = dist;
                result = closestPoint;
                snapped = true;
                m_measureSnappedToLine = true;
                m_measureSnappedLineIndex = i;
                m_measureSnappedToRobot = false;
            }
        }
    }

    return result;
}

// Get minimum movement distance for line drawing (stabilization)
double MapCanvas::getMinimumMoveDistance() const {
    // Minimum distance = 5 pixels in world coordinates
    // This reduces sensitivity and jitter when drawing
    return 5.0 / m_scale;
}

// Clear all selections
void MapCanvas::clearSelection() {
    m_selectedLineIndex = -1;
    m_selectedPathIndex = -1;
    m_selectedWaypointIndex = -1;
}

// Delete the currently selected line
void MapCanvas::deleteSelectedLine() {
    if (!m_mapData || m_selectedLineIndex < 0 || m_selectedLineIndex >= m_mapData->lines.size()) {
        return;
    }

    m_mapData->removeLine(m_selectedLineIndex);
    m_selectedLineIndex = -1;
    emit statusMessage(QString("Line deleted"));
    update();
}

// Delete the currently selected waypoint
void MapCanvas::deleteSelectedWaypoint() {
    if (!m_pathCollection || m_selectedPathIndex < 0 || m_selectedWaypointIndex < 0) {
        return;
    }

    if (m_selectedPathIndex >= m_pathCollection->paths.size()) {
        return;
    }

    PathData& path = m_pathCollection->paths[m_selectedPathIndex];
    if (m_selectedWaypointIndex >= path.waypoints.size()) {
        return;
    }

    path.removeWaypoint(m_selectedWaypointIndex);
    m_selectedPathIndex = -1;
    m_selectedWaypointIndex = -1;
    emit statusMessage(QString("Waypoint deleted"));
    update();
}

// Handle keyboard events for deletion
void MapCanvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // Delete selected line
        if (m_selectedLineIndex >= 0) {
            deleteSelectedLine();
            return;
        }

        // Delete selected waypoint
        if (m_selectedPathIndex >= 0 && m_selectedWaypointIndex >= 0) {
            deleteSelectedWaypoint();
            return;
        }

        emit statusMessage(QString("Nothing selected to delete"));
    }
    else if (event->key() == Qt::Key_F) {
        m_fixedSnapEnabled = !m_fixedSnapEnabled;
        emit statusMessage(QString("Fixed snap %1 (%.2fm / %2°)")
            .arg(m_fixedSnapEnabled ? "ON" : "OFF")
            .arg(m_fixedLengthStep, 0, 'f', 2)
            .arg(m_fixedAngleStep, 0, 'f', 0));
        return;
    }
    else if (event->key() == Qt::Key_Escape) {
        // Cancel operations and clear selection
        m_isDrawing = false;
        m_waitingForSecondClick = false;
        m_constrainedDrawing = false;
        m_measuring = false;
        m_measureLocked = false;
        m_isDraggingRobot = false;
        m_isDraggingWaypoint = false;
        m_isEditingHeading = false;
        clearSelection();
        emit statusMessage(QString("Operation cancelled"));
        update();
    }
    else if (event->key() == Qt::Key_D && m_waitingForSecondClick) {
        // Press 'D' while drawing to enter exact dimensions
        promptForDimensions();
    }
    else if (event->key() == Qt::Key_E && m_selectedLineIndex >= 0) {
        // Press 'E' to edit selected line properties
        editLineProperties(m_selectedLineIndex);
    }

    QWidget::keyPressEvent(event);
}

// Handle context menu (right-click)
void MapCanvas::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);

    Geometry::Point worldPos = screenToWorld(event->pos());

    // Check if clicking on a waypoint
    int pathIdx = -1, wpIdx = -1;
    if (findNearestWaypoint(worldPos, pathIdx, wpIdx)) {
        m_selectedPathIndex = pathIdx;
        m_selectedWaypointIndex = wpIdx;
        m_selectedLineIndex = -1;

        menu.addAction("Delete Waypoint", this, &MapCanvas::deleteSelectedWaypoint);
        menu.addAction("Edit Waypoint Properties", [this, pathIdx, wpIdx]() {
            emit waypointDoubleClicked(pathIdx, wpIdx);
        });
        menu.addSeparator();
        menu.exec(event->globalPos());
        return;
    }

    // Check if clicking on a line
    if (m_mapData) {
        int lineIdx = findNearestLine(worldPos, LINE_PROXIMITY_THRESHOLD);
        if (lineIdx >= 0) {
            m_selectedLineIndex = lineIdx;
            m_selectedPathIndex = -1;
            m_selectedWaypointIndex = -1;

            menu.addAction("Edit Line Properties", [this, lineIdx]() {
                emit lineDoubleClicked(lineIdx);
            });
            menu.addAction("Delete Line", this, &MapCanvas::deleteSelectedLine);
            menu.addSeparator();
            menu.exec(event->globalPos());
            return;
        }
    }

    // General context menu (no selection)
    clearSelection();
    menu.addAction("Clear Selection", this, &MapCanvas::clearSelection);
    menu.exec(event->globalPos());
}

// CAD-style: Prompt for exact distance and angle
void MapCanvas::promptForDimensions() {
    if (!m_waitingForSecondClick) {
        return;
    }

    QString input;
    if (showStyledTextInput("Enter Dimensions",
                            "Format: distance,angle\nExample: 2.0,90\n(distance in meters, angle in degrees)",
                            QString("%1,%2").arg(m_constrainedDistance).arg(m_constrainedAngle),
                            input) && !input.isEmpty()) {
        QStringList parts = input.split(',');
        if (parts.size() == 2) {
            m_constrainedDistance = parts[0].toDouble();
            m_constrainedAngle = parts[1].toDouble();
            m_constrainedDrawing = true;

            applyConstrainedDimensions();

            emit statusMessage(QString("Constrained: %1 m @ %2° - Click to place or press D to adjust")
                .arg(m_constrainedDistance, 0, 'f', 3)
                .arg(m_constrainedAngle, 0, 'f', 1));

            update();
        } else {
            emit statusMessage(QString("Invalid format! Use: distance,angle (e.g., 2.0,90)"));
        }
    }
}

// Apply constrained dimensions to current drawing
void MapCanvas::applyConstrainedDimensions() {
    if (!m_constrainedDrawing) {
        return;
    }

    // Convert angle to radians (CAD: 0° = East, 90° = North)
    double angleRad = m_constrainedAngle * M_PI / 180.0;

    // Calculate endpoint from start point
    double endX = m_drawStartPoint.x + m_constrainedDistance * std::cos(angleRad);
    double endY = m_drawStartPoint.y + m_constrainedDistance * std::sin(angleRad);

    m_drawCurrentPoint = Geometry::Point(endX, endY);
    m_lastDrawPoint = m_drawCurrentPoint;
}

// Edit properties of an existing line
void MapCanvas::editLineProperties(int lineIndex) {
    if (!m_mapData || lineIndex < 0 || lineIndex >= m_mapData->lines.size()) {
        return;
    }

    Geometry::Line& line = m_mapData->lines[lineIndex];
    double currentLength = line.length();
    double currentAngle = line.angleDegrees();

    QString input;
    if (showStyledTextInput("Edit Line Properties",
                             QString("Current: %1 m @ %2°\n\nEnter new dimensions:\nFormat: distance,angle\nExample: 2.0,90")
                                 .arg(currentLength, 0, 'f', 3)
                                 .arg(currentAngle, 0, 'f', 1),
                             QString("%1,%2").arg(currentLength, 0, 'f', 3).arg(currentAngle, 0, 'f', 1),
                             input) && !input.isEmpty()) {
        QStringList parts = input.split(',');
        if (parts.size() == 2) {
            double newDistance = parts[0].toDouble();
            double newAngle = parts[1].toDouble();

            // Keep start point, recalculate end point
            double angleRad = newAngle * M_PI / 180.0;
            line.end.x = line.start.x + newDistance * std::cos(angleRad);
            line.end.y = line.start.y + newDistance * std::sin(angleRad);

            emit statusMessage(QString("Line updated: %1 m @ %2°")
                .arg(newDistance, 0, 'f', 3)
                .arg(newAngle, 0, 'f', 1));

            update();
        } else {
            emit statusMessage(QString("Invalid format! Use: distance,angle"));
        }
    }
}

bool MapCanvas::showStyledTextInput(const QString& title, const QString& prompt,
                                    const QString& defaultValue, QString& outValue) const {
    QInputDialog dialog;
    dialog.setWindowTitle(title);
    dialog.setLabelText(prompt);
    dialog.setTextValue(defaultValue);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.resize(420, 0);

    QPalette pal = dialog.palette();
    pal.setColor(QPalette::Window, QColor(20, 25, 35));
    pal.setColor(QPalette::Base, QColor(15, 20, 30));
    pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    dialog.setPalette(pal);

    dialog.setStyleSheet(
        "QLabel { color: white; font-weight: 600; }"
        "QLineEdit { color: white; background: #101622; border: 1px solid #3c4f6b; padding: 4px; }"
        "QPushButton { color: white; background-color: #1f2b3d; padding: 4px 12px; border: 1px solid #3c4f6b; }"
        "QPushButton:hover { background-color: #27354a; }"
    );

    if (dialog.exec() == QDialog::Accepted) {
        outValue = dialog.textValue();
        return true;
    }
    return false;
}

Geometry::Point MapCanvas::applyFixedLengthAngleSnap(const Geometry::Point& start,
                                                     const Geometry::Point& candidate) const {
    Geometry::Point snapped = candidate;
    double dx = candidate.x - start.x;
    double dy = candidate.y - start.y;
    double length = std::sqrt(dx * dx + dy * dy);

    if (length < 1e-6) {
        return snapped;
    }

    double angleDeg = std::atan2(dy, dx) * 180.0 / M_PI;
    double snappedAngleDeg = std::round(angleDeg / m_fixedAngleStep) * m_fixedAngleStep;
    double snappedLength = std::max(m_fixedLengthStep,
                                    std::round(length / m_fixedLengthStep) * m_fixedLengthStep);
    double snappedAngleRad = snappedAngleDeg * M_PI / 180.0;

    snapped.x = start.x + snappedLength * std::cos(snappedAngleRad);
    snapped.y = start.y + snappedLength * std::sin(snappedAngleRad);
    return snapped;
}
