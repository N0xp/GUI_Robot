#ifndef MAPCANVAS_H
#define MAPCANVAS_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "Geometry.h"
#include "MapData.h"
#include "PathData.h"

class MapCanvas : public QWidget {
    Q_OBJECT

public:
    enum class Tool {
        None,
        DrawLine,
        DrawPath,
        Select,
        Measure,
        Pan,
        AddReference
    };

    explicit MapCanvas(QWidget* parent = nullptr);

    // Set data
    void setMapData(MapData* mapData);
    void setPathCollection(PathCollection* paths);
    void setRobotPose(const Geometry::RobotPose& pose);
    Geometry::RobotPose getCurrentPose() const;

    // Multiple robots
    void addRobot(const Geometry::RobotPose& pose);
    void removeRobot(int index);
    void clearRobots();
    int getRobotCount() const { return m_robots.size(); }
    void updateRobotPose(int index, const Geometry::RobotPose& pose);
    void setPrimaryRobot(int index) { m_primaryRobotIndex = index; }

    // Tool selection
    void setTool(Tool tool);
    Tool currentTool() const { return m_currentTool; }

    // View control
    void zoomIn();
    void zoomOut();
    void resetView();
    void fitToView();

    // Grid settings
    void setShowGrid(bool show) { m_showGrid = show; update(); }
    void setShowDimensions(bool show) { m_showDimensions = show; update(); }
    void setShowRobot(bool show) { m_showRobot = show; update(); }
    void setSnapToPoints(bool snap) { m_snapToPoints = snap; }
    void setSnapDistance(double dist) { m_snapDistance = dist; }

    // Coordinate conversion
    QPointF worldToScreen(const Geometry::Point& worldPoint) const;
    Geometry::Point screenToWorld(const QPointF& screenPoint) const;

signals:
    void lineAdded(const Geometry::Line& line);
    void waypointAdded(const Geometry::Waypoint& waypoint);
    void referencePointAdded(const Geometry::ReferencePoint& refPoint);
    void selectionChanged(int index);
    void distanceMeasured(double distance);
    void statusMessage(const QString& message);
    void lineDoubleClicked(int lineIndex);
    void cursorPositionChanged(double x, double y);
    void waypointDoubleClicked(int pathIndex, int waypointIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void drawGrid(QPainter& painter);
    void drawOrigin(QPainter& painter);
    void drawLines(QPainter& painter);
    void drawPaths(QPainter& painter);
    void drawReferencePoints(QPainter& painter);
    void drawRobot(QPainter& painter, const Geometry::RobotPose& pose);
    void drawDimension(QPainter& painter, const Geometry::Line& line);
    void drawAngle(QPainter& painter, const Geometry::Line& line);
    void drawMeasurement(QPainter& painter);

    // Drawing helpers
    void drawRobotRectangle(QPainter& painter, const QPointF& center, double width, double length, double heading);
    void drawRobotSquare(QPainter& painter, const QPointF& center, double size, double heading);
    void drawRobotTriangle(QPainter& painter, const QPointF& center, double size, double heading);

    // Handle different tools
    void handleDrawLinePress(const QPointF& pos);
    void handleDrawLineMove(const QPointF& pos);
    void handleDrawLineRelease(const QPointF& pos);

    void handleDrawPathPress(const QPointF& pos);
    void handleMeasurePress(const QPointF& pos);
    void handleMeasureMove(const QPointF& pos);
    void handleAddReferencePress(const QPointF& pos);

    // Helper functions
    Geometry::Point snapToNearestPoint(const Geometry::Point& point);
    Geometry::Point snapToNearestLineOrRobot(const Geometry::Point& point, bool& snapped);
    QVector<Geometry::Point> getAllSnappablePoints() const;
    int findNearestRobot(const Geometry::Point& point, double maxDistance = 0.5);
    bool findNearestWaypoint(const Geometry::Point& point, int& pathIdx, int& wpIdx, double maxDistance = 0.3);
    bool isNearWaypointHeadingHandle(const Geometry::Point& point, int pathIdx, int wpIdx);
    int findNearestLine(const Geometry::Point& point, double maxDistance = 0.2);
    Geometry::Point getClosestPointOnLine(const Geometry::Line& line, const Geometry::Point& point) const;
    double getMinimumMoveDistance() const; // Minimum distance before updating drawing

    // CAD-style dimension input
    void promptForDimensions();
    void applyConstrainedDimensions();
    void editLineProperties(int lineIndex);
    bool showStyledTextInput(const QString& title, const QString& prompt,
                             const QString& defaultValue, QString& outValue) const;
    Geometry::Point applyFixedLengthAngleSnap(const Geometry::Point& start,
                                              const Geometry::Point& candidate) const;

    MapData* m_mapData;
    PathCollection* m_pathCollection;
    QVector<Geometry::RobotPose> m_robots;  // Support multiple robots
    int m_primaryRobotIndex;  // Which robot receives updates from RobotComm

    // View transform
    QPointF m_viewOffset; // Screen offset
    double m_scale; // Pixels per meter
    double m_minScale;
    double m_maxScale;

    // Drawing state
    Tool m_currentTool;
    bool m_isDrawing;
    bool m_waitingForSecondClick; // SolidWorks-style: true after first click, waiting for second
    bool m_constrainedDrawing; // CAD-style: constrain to exact distance/angle
    double m_constrainedDistance; // meters
    double m_constrainedAngle; // degrees
    Geometry::Point m_drawStartPoint;
    Geometry::Point m_drawCurrentPoint;
    Geometry::Point m_lastDrawPoint; // For stabilization
    Geometry::Point m_cursorWorldPos; // Current cursor position in world coordinates

    // Measurement
    Geometry::Point m_measureStart;
    Geometry::Point m_measureEnd;
    bool m_measuring;
    bool m_measureLocked;
    bool m_measureSnappedToLine;  // Track if snapped to line
    bool m_measureSnappedToRobot; // Track if snapped to robot
    int m_measureSnappedLineIndex; // Which line we snapped to
    int m_measureSnappedRobotIndex; // Which robot we snapped to

    // Display options
    bool m_showGrid;
    bool m_showDimensions;
    bool m_showRobot;

    // Snapping
    bool m_snapToPoints;
    double m_snapDistance; // in meters

    // Robot dragging
    bool m_isDraggingRobot;
    int m_draggedRobotIndex;

    // Waypoint dragging (move/rotate)
    bool m_isDraggingWaypoint;
    int m_draggedPathIndex;
    int m_draggedWaypointIndex;

    // Waypoint heading editing
    bool m_isEditingHeading;
    int m_editingPathIndex;
    int m_editingWaypointIndex;

    // Line editing
    int m_selectedLineIndex;
    bool m_isDraggingLineEndpoint;
    bool m_draggingLineStart;  // true = start point, false = end point

    // Selection tracking
    int m_selectedPathIndex;
    int m_selectedWaypointIndex;

    // Helper methods for deletion
    void deleteSelectedLine();
    void deleteSelectedWaypoint();
    void clearSelection();

    // Mouse state
    QPointF m_lastMousePos;
    bool m_isPanning;

    // Fixed snapping
    bool m_fixedSnapEnabled;
    double m_fixedLengthStep;
    double m_fixedAngleStep;
};

#endif // MAPCANVAS_H
