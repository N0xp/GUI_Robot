#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QToolBar>
#include <QAction>

#include "MapCanvas.h"
#include "MapData.h"
#include "PathData.h"
#include "RobotComm.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu
    void newMap();
    void openMap();
    void saveMap();
    void saveMapAs();
    void exportMap();

    // Tools
    void selectDrawLineTool();
    void selectDrawPathTool();
    void selectSelectTool();
    void selectMeasureTool();
    void selectPanTool();
    void selectAddReferenceTool();

    // View
    void zoomIn();
    void zoomOut();
    void resetView();
    void fitToView();

    // Robot
    void connectToRobot();
    void disconnectFromRobot();
    void sendCurrentPathToRobot();
    void sendMapToRobot();
    void sendReferencePointsToRobot();
    void onRobotConnected();
    void onRobotDisconnected();
    void onRobotPoseUpdated(const Geometry::RobotPose& pose);

    // Paths
    void createNewPath();
    void deletePath();
    void duplicatePath();
    void onPathSelectionChanged();
    void saveAllPaths();
    void loadPaths();
    void editWaypoint(int pathIndex, int waypointIndex);

    // Robot shape
    void onRobotShapeChanged(int index);
    void onRobotSizeChanged();

    // Settings
    void onGridSizeChanged(double size);

    // Status updates
    void updateStatusMessage(const QString& message);
    void updateRobotStatus();

private:
    void setupUI();
    void createMenus();
    void createToolbars();
    void createDockWidgets();
    void createStatusBar();
    void setupConnections();
    void updateToolButtonSelection(QAction* activeAction);
    void markMapModified();
    void refreshWindowTitle();
    QString dialogDirectory(const QString& key) const;
    void setDialogDirectory(const QString& key, const QString& filePath) const;
    QString defaultDialogDirectory() const;

    // UI Components
    MapCanvas* m_canvas;
    QToolBar* m_fileToolbar = nullptr;
    QToolBar* m_viewToolbar = nullptr;
    QToolBar* m_toolsToolbar = nullptr;
    QToolBar* m_robotToolbar = nullptr;
    QToolBar* m_pathsToolbar = nullptr;

    QAction* m_selectToolAction = nullptr;
    QAction* m_drawLineToolAction = nullptr;
    QAction* m_drawPathToolAction = nullptr;
    QAction* m_measureToolAction = nullptr;
    QAction* m_panToolAction = nullptr;
    QAction* m_addReferenceToolAction = nullptr;

    // Dock widgets
    QDockWidget* m_pathsDock;
    QDockWidget* m_robotDock;
    QDockWidget* m_settingsDock;

    // Path panel
    QListWidget* m_pathList;
    QPushButton* m_newPathBtn;
    QPushButton* m_deletePathBtn;
    QPushButton* m_duplicatePathBtn;
    QPushButton* m_sendPathBtn;
    QLabel* m_pathLengthLabel;

    // Robot panel
    QLabel* m_connectionStatusLabel;
    QPushButton* m_connectBtn;
    QLineEdit* m_robotIpEdit;
    QLabel* m_robotPositionLabel;
    QLabel* m_robotHeadingLabel;
    QComboBox* m_robotShapeCombo;
    QDoubleSpinBox* m_robotWidthSpin;
    QDoubleSpinBox* m_robotLengthSpin;

    // Settings panel
    QDoubleSpinBox* m_gridSizeSpin;
    QLabel* m_originLabel;  // Shows origin is fixed at (0,0)

    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_coordsLabel;
    QLabel* m_robotStatusLabel;

    // Data
    MapData m_mapData;
    PathCollection m_pathCollection;
    RobotComm* m_robotComm;

    QString m_currentMapFile;
    bool m_mapModified;
};

#endif // MAINWINDOW_H
