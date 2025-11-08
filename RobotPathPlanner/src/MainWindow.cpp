#include "MainWindow.h"
#include "WaypointDialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QInputDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QActionGroup>
#include <QList>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QKeySequence>
#include <QDebug>

namespace {
    constexpr char kSettingsOrganization[] = "UAE";
    constexpr char kSettingsApplication[] = "RobotPathPlanner";
    constexpr char kMapDirectoryKey[] = "directories/map";
    constexpr char kPathsDirectoryKey[] = "directories/paths";
}

void MainWindow::createToolbars() {
    // File toolbar
    m_fileToolbar = addToolBar("File");
    m_fileToolbar->setObjectName("FileToolbar");
    m_fileToolbar->setMovable(false);
    m_fileToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_fileToolbar->addAction("New Map", this, &MainWindow::newMap);
    m_fileToolbar->addAction("Open Map", this, &MainWindow::openMap);
    m_fileToolbar->addAction("Save Map", this, &MainWindow::saveMap);
    m_fileToolbar->addAction("Save As", this, &MainWindow::saveMapAs);
    m_fileToolbar->addAction("Export Map", this, &MainWindow::exportMap);

    // View toolbar
    m_viewToolbar = addToolBar("View");
    m_viewToolbar->setObjectName("ViewToolbar");
    m_viewToolbar->setMovable(false);
    m_viewToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_viewToolbar->addAction("Zoom In", this, &MainWindow::zoomIn);
    m_viewToolbar->addAction("Zoom Out", this, &MainWindow::zoomOut);
    m_viewToolbar->addAction("Reset View", this, &MainWindow::resetView);
    m_viewToolbar->addAction("Fit to View", this, &MainWindow::fitToView);

    addToolBarBreak();

    // Tools toolbar with exclusive selection
    m_toolsToolbar = addToolBar("Tools");
    m_toolsToolbar->setObjectName("ToolsToolbar");
    m_toolsToolbar->setMovable(false);
    m_toolsToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    m_selectToolAction = new QAction("Select", this);
    m_selectToolAction->setCheckable(true);
    toolGroup->addAction(m_selectToolAction);
    connect(m_selectToolAction, &QAction::triggered, this, &MainWindow::selectSelectTool);
    m_toolsToolbar->addAction(m_selectToolAction);

    m_drawLineToolAction = new QAction("Draw Line", this);
    m_drawLineToolAction->setCheckable(true);
    toolGroup->addAction(m_drawLineToolAction);
    connect(m_drawLineToolAction, &QAction::triggered, this, &MainWindow::selectDrawLineTool);
    m_toolsToolbar->addAction(m_drawLineToolAction);

    m_drawPathToolAction = new QAction("Draw Path", this);
    m_drawPathToolAction->setCheckable(true);
    toolGroup->addAction(m_drawPathToolAction);
    connect(m_drawPathToolAction, &QAction::triggered, this, &MainWindow::selectDrawPathTool);
    m_toolsToolbar->addAction(m_drawPathToolAction);

    m_measureToolAction = new QAction("Measure", this);
    m_measureToolAction->setCheckable(true);
    toolGroup->addAction(m_measureToolAction);
    connect(m_measureToolAction, &QAction::triggered, this, &MainWindow::selectMeasureTool);
    m_toolsToolbar->addAction(m_measureToolAction);

    m_panToolAction = new QAction("Pan", this);
    m_panToolAction->setCheckable(true);
    toolGroup->addAction(m_panToolAction);
    connect(m_panToolAction, &QAction::triggered, this, &MainWindow::selectPanTool);
    m_toolsToolbar->addAction(m_panToolAction);

    m_addReferenceToolAction = new QAction("Add Ref", this);
    m_addReferenceToolAction->setCheckable(true);
    toolGroup->addAction(m_addReferenceToolAction);
    connect(m_addReferenceToolAction, &QAction::triggered, this, &MainWindow::selectAddReferenceTool);
    m_toolsToolbar->addAction(m_addReferenceToolAction);

    // Default selection
    m_selectToolAction->setChecked(true);
    updateToolButtonSelection(m_selectToolAction);

    addToolBarBreak();

    // Robot toolbar
    m_robotToolbar = addToolBar("Robot");
    m_robotToolbar->setObjectName("RobotToolbar");
    m_robotToolbar->setMovable(false);
    m_robotToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_robotToolbar->addAction("Connect", this, &MainWindow::connectToRobot);
    m_robotToolbar->addAction("Disconnect", this, &MainWindow::disconnectFromRobot);
    m_robotToolbar->addSeparator();
    m_robotToolbar->addAction("Send Path", this, &MainWindow::sendCurrentPathToRobot);
    m_robotToolbar->addAction("Send Map", this, &MainWindow::sendMapToRobot);
    m_robotToolbar->addAction("Send Reference", this, &MainWindow::sendReferencePointsToRobot);

    addToolBarBreak();

    // Paths toolbar
    m_pathsToolbar = addToolBar("Paths");
    m_pathsToolbar->setObjectName("PathsToolbar");
    m_pathsToolbar->setMovable(false);
    m_pathsToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_pathsToolbar->addAction("New Path", this, &MainWindow::createNewPath);
    m_pathsToolbar->addAction("Delete Path", this, &MainWindow::deletePath);
    m_pathsToolbar->addAction("Duplicate Path", this, &MainWindow::duplicatePath);
    m_pathsToolbar->addSeparator();
    m_pathsToolbar->addAction("Save Paths", this, &MainWindow::saveAllPaths);
    m_pathsToolbar->addAction("Load Paths", this, &MainWindow::loadPaths);

    m_canvas->setTool(MapCanvas::Tool::Select);
}

void MainWindow::updateToolButtonSelection(QAction* activeAction) {
    const QList<QAction*> toolActions = {
        m_selectToolAction,
        m_drawLineToolAction,
        m_drawPathToolAction,
        m_measureToolAction,
        m_panToolAction,
        m_addReferenceToolAction
    };

    for (QAction* action : toolActions) {
        if (action) {
            action->setChecked(action == activeAction);
        }
    }
}
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_canvas(new MapCanvas(this))
    , m_robotComm(new RobotComm(this))
    , m_mapModified(false)
{
    setWindowTitle("Robot Path Planner");
    resize(1200, 800);

    setupUI();
    setupConnections();

    // Initialize with default map and one path
    m_mapData.name = "New Map";
    m_mapData.gridSize = 1.0;
    m_pathCollection.addPath(PathData("Path 1"));

    m_canvas->setMapData(&m_mapData);
    m_canvas->setPathCollection(&m_pathCollection);
    m_canvas->resetView();

    updateRobotStatus();
    refreshWindowTitle();
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_mapModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            saveMap();
            // Only accept close if save succeeded (m_mapModified will be false)
            if (m_mapModified) {
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        // If Discard, just fall through and accept the close
    }

    event->accept();
}

void MainWindow::setupUI() {
    createMenus();
    createToolbars();
    setCentralWidget(m_canvas);
    createDockWidgets();
    createStatusBar();
}

void MainWindow::createMenus() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* newMapAction = fileMenu->addAction("New Map", this, &MainWindow::newMap);
    newMapAction->setShortcut(QKeySequence::New);

    QAction* openMapAction = fileMenu->addAction("Open Map...", this, &MainWindow::openMap);
    openMapAction->setShortcut(QKeySequence::Open);

    QAction* saveMapAction = fileMenu->addAction("Save Map", this, &MainWindow::saveMap);
    saveMapAction->setShortcut(QKeySequence::Save);

    QAction* saveMapAsAction = fileMenu->addAction("Save Map As...", this, &MainWindow::saveMapAs);
    saveMapAsAction->setShortcut(QKeySequence::SaveAs);

    fileMenu->addSeparator();

    fileMenu->addAction("Export Map...", this, &MainWindow::exportMap);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("Exit", this, &QWidget::close);
    exitAction->setShortcut(QKeySequence::Quit);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu("View");

    viewMenu->addAction("Zoom In", this, &MainWindow::zoomIn);
    viewMenu->addAction("Zoom Out", this, &MainWindow::zoomOut);
    viewMenu->addAction("Reset View", this, &MainWindow::resetView);
    viewMenu->addAction("Fit to View", this, &MainWindow::fitToView);

    // Robot menu
    QMenu* robotMenu = menuBar()->addMenu("Robot");

    robotMenu->addAction("Connect to Robot", this, &MainWindow::connectToRobot);
    robotMenu->addAction("Disconnect", this, &MainWindow::disconnectFromRobot);

    robotMenu->addSeparator();

    robotMenu->addAction("Send Path to Robot", this, &MainWindow::sendCurrentPathToRobot);
    robotMenu->addAction("Send Map to Robot", this, &MainWindow::sendMapToRobot);
    robotMenu->addAction("Send Reference Points to Robot", this, &MainWindow::sendReferencePointsToRobot);
}

void MainWindow::markMapModified() {
    if (!m_mapModified) {
        m_mapModified = true;
        refreshWindowTitle();
    }
}

void MainWindow::refreshWindowTitle() {
    QString title = "Robot Path Planner - ";
    if (!m_currentMapFile.isEmpty()) {
        QFileInfo info(m_currentMapFile);
        title += info.fileName();
    } else {
        title += m_mapData.name;
    }

    if (m_mapModified) {
        title += " *";
    }

    setWindowTitle(title);
}

QString MainWindow::dialogDirectory(const QString& key) const {
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    QString path = settings.value(key).toString();
    if (path.isEmpty()) {
        path = defaultDialogDirectory();
    }
    return path;
}

void MainWindow::setDialogDirectory(const QString& key, const QString& filePath) const {
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo info(filePath);
    QString dirPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    if (dirPath.isEmpty()) {
        return;
    }

    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.setValue(key, dirPath);
}

QString MainWindow::defaultDialogDirectory() const {
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents.isEmpty()) {
        return documents;
    }
    return QDir::homePath();
}


void MainWindow::createDockWidgets() {
    // Paths dock
    m_pathsDock = new QDockWidget("Paths", this);
    QWidget* pathsWidget = new QWidget;
    QVBoxLayout* pathsLayout = new QVBoxLayout(pathsWidget);

    m_pathList = new QListWidget;
    pathsLayout->addWidget(m_pathList);

    QHBoxLayout* pathButtonsLayout = new QHBoxLayout;
    m_newPathBtn = new QPushButton("New");
    m_deletePathBtn = new QPushButton("Delete");
    m_duplicatePathBtn = new QPushButton("Duplicate");
    m_sendPathBtn = new QPushButton("Send to Robot");

    pathButtonsLayout->addWidget(m_newPathBtn);
    pathButtonsLayout->addWidget(m_deletePathBtn);
    pathButtonsLayout->addWidget(m_duplicatePathBtn);
    pathsLayout->addLayout(pathButtonsLayout);
    pathsLayout->addWidget(m_sendPathBtn);

    m_pathLengthLabel = new QLabel("Length: 0.0 m");
    pathsLayout->addWidget(m_pathLengthLabel);

    QPushButton* savePathsBtn = new QPushButton("Save All Paths");
    QPushButton* loadPathsBtn = new QPushButton("Load Paths");
    pathsLayout->addWidget(savePathsBtn);
    pathsLayout->addWidget(loadPathsBtn);

    connect(savePathsBtn, &QPushButton::clicked, this, &MainWindow::saveAllPaths);
    connect(loadPathsBtn, &QPushButton::clicked, this, &MainWindow::loadPaths);

    m_pathsDock->setWidget(pathsWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_pathsDock);

    // Robot dock
    m_robotDock = new QDockWidget("Robot", this);
    QWidget* robotWidget = new QWidget;
    QVBoxLayout* robotLayout = new QVBoxLayout(robotWidget);

    QGroupBox* connectionGroup = new QGroupBox("Connection");
    QVBoxLayout* connLayout = new QVBoxLayout(connectionGroup);

    m_connectionStatusLabel = new QLabel("Disconnected");
    m_connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    connLayout->addWidget(m_connectionStatusLabel);

    QHBoxLayout* ipLayout = new QHBoxLayout;
    ipLayout->addWidget(new QLabel("Robot IP:"));
    m_robotIpEdit = new QLineEdit("10.0.0.2");
    ipLayout->addWidget(m_robotIpEdit);
    connLayout->addLayout(ipLayout);

    m_connectBtn = new QPushButton("Connect");
    connLayout->addWidget(m_connectBtn);

    robotLayout->addWidget(connectionGroup);

    QGroupBox* statusGroup = new QGroupBox("Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

    m_robotPositionLabel = new QLabel("Position: (0.0, 0.0)");
    m_robotHeadingLabel = new QLabel("Heading: 0.0°");
    statusLayout->addWidget(m_robotPositionLabel);
    statusLayout->addWidget(m_robotHeadingLabel);

    robotLayout->addWidget(statusGroup);

    QGroupBox* shapeGroup = new QGroupBox("Robot Shape");
    QFormLayout* shapeLayout = new QFormLayout(shapeGroup);

    m_robotShapeCombo = new QComboBox;
    m_robotShapeCombo->addItem("Rectangle");
    m_robotShapeCombo->addItem("Square");
    m_robotShapeCombo->addItem("Triangle");
    shapeLayout->addRow("Shape:", m_robotShapeCombo);

    m_robotWidthSpin = new QDoubleSpinBox;
    m_robotWidthSpin->setRange(0.1, 5.0);
    m_robotWidthSpin->setValue(0.6);
    m_robotWidthSpin->setSuffix(" m");
    m_robotWidthSpin->setSingleStep(0.05);
    shapeLayout->addRow("Width:", m_robotWidthSpin);

    m_robotLengthSpin = new QDoubleSpinBox;
    m_robotLengthSpin->setRange(0.1, 5.0);
    m_robotLengthSpin->setValue(0.8);
    m_robotLengthSpin->setSuffix(" m");
    m_robotLengthSpin->setSingleStep(0.05);
    shapeLayout->addRow("Length:", m_robotLengthSpin);

    robotLayout->addWidget(shapeGroup);

    // Multiple robots section
    QGroupBox* multiRobotGroup = new QGroupBox("Multiple Robots");
    QVBoxLayout* multiRobotLayout = new QVBoxLayout(multiRobotGroup);

    QLabel* infoLabel = new QLabel("Drag robots on map to reposition");
    infoLabel->setStyleSheet("color: gray; font-style: italic;");
    multiRobotLayout->addWidget(infoLabel);

    QHBoxLayout* robotBtnLayout = new QHBoxLayout;
    QPushButton* addRobotBtn = new QPushButton("Add Robot");
    QPushButton* removeRobotBtn = new QPushButton("Remove Last");

    robotBtnLayout->addWidget(addRobotBtn);
    robotBtnLayout->addWidget(removeRobotBtn);
    multiRobotLayout->addLayout(robotBtnLayout);

    QLabel* robotCountLabel = new QLabel("Robots: 1");
    multiRobotLayout->addWidget(robotCountLabel);

    robotLayout->addWidget(multiRobotGroup);

    // Connect multi-robot buttons
    connect(addRobotBtn, &QPushButton::clicked, [this, robotCountLabel]() {
        Geometry::RobotPose newRobot;
        newRobot.position = Geometry::Point(1.0, 1.0); // Default position
        m_canvas->addRobot(newRobot);
        robotCountLabel->setText(QString("Robots: %1").arg(m_canvas->getRobotCount()));
        statusBar()->showMessage(QString("Added robot #%1 - drag it to position").arg(m_canvas->getRobotCount()), 3000);
    });

    connect(removeRobotBtn, &QPushButton::clicked, [this, robotCountLabel]() {
        int count = m_canvas->getRobotCount();
        if (count > 1) {
            m_canvas->removeRobot(count - 1);
            robotCountLabel->setText(QString("Robots: %1").arg(m_canvas->getRobotCount()));
            statusBar()->showMessage("Removed last robot", 2000);
        } else {
            statusBar()->showMessage("Cannot remove the only robot", 2000);
        }
    });

    // Send Data to Robot section
    QGroupBox* sendDataGroup = new QGroupBox("Send Data to Robot");
    QVBoxLayout* sendDataLayout = new QVBoxLayout(sendDataGroup);

    QPushButton* sendPathBtn = new QPushButton("Send Path");
    sendPathBtn->setToolTip("Send current path to robot");
    sendPathBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    sendDataLayout->addWidget(sendPathBtn);

    QPushButton* sendMapBtn = new QPushButton("Send Map");
    sendMapBtn->setToolTip("Send map data (lines) to robot");
    sendMapBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; }");
    sendDataLayout->addWidget(sendMapBtn);

    QPushButton* sendRefPointsBtn = new QPushButton("Send Reference Points");
    sendRefPointsBtn->setToolTip("Send reference points to robot");
    sendRefPointsBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 8px; }");
    sendDataLayout->addWidget(sendRefPointsBtn);

    robotLayout->addWidget(sendDataGroup);

    // Connect send data buttons
    connect(sendPathBtn, &QPushButton::clicked, this, &MainWindow::sendCurrentPathToRobot);
    connect(sendMapBtn, &QPushButton::clicked, this, &MainWindow::sendMapToRobot);
    connect(sendRefPointsBtn, &QPushButton::clicked, this, &MainWindow::sendReferencePointsToRobot);

    robotLayout->addStretch();

    m_robotDock->setWidget(robotWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_robotDock);

    // Settings dock
    m_settingsDock = new QDockWidget("Settings", this);
    QWidget* settingsWidget = new QWidget;
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsWidget);

    QGroupBox* mapGroup = new QGroupBox("Map Settings");
    QFormLayout* mapLayout = new QFormLayout(mapGroup);

    m_gridSizeSpin = new QDoubleSpinBox;
    m_gridSizeSpin->setRange(0.1, 10.0);
    m_gridSizeSpin->setValue(1.0);
    m_gridSizeSpin->setSuffix(" m");
    m_gridSizeSpin->setSingleStep(0.1);
    mapLayout->addRow("Grid Size:", m_gridSizeSpin);

    // Origin is fixed at (0,0) as global coordinate reference
    m_originLabel = new QLabel("<b>Origin (0,0):</b> Global Reference");
    m_originLabel->setStyleSheet("color: red; font-weight: bold;");
    mapLayout->addRow(m_originLabel);

    settingsLayout->addWidget(mapGroup);
    settingsLayout->addStretch();

    m_settingsDock->setWidget(settingsWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_settingsDock);
}

void MainWindow::createStatusBar() {
    m_statusLabel = new QLabel("Ready");
    m_coordsLabel = new QLabel("X: 0.0 m, Y: 0.0 m");
    m_robotStatusLabel = new QLabel("Robot: Disconnected");

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_coordsLabel);
    statusBar()->addPermanentWidget(m_robotStatusLabel);
}

void MainWindow::setupConnections() {
    // Canvas signals
    connect(m_canvas, &MapCanvas::statusMessage, this, &MainWindow::updateStatusMessage);
    connect(m_canvas, &MapCanvas::lineAdded, this, [this](const Geometry::Line&) {
        markMapModified();
        m_canvas->update();
    });
    connect(m_canvas, &MapCanvas::waypointAdded, this, [this](const Geometry::Waypoint&) {
        markMapModified();
    });
    connect(m_canvas, &MapCanvas::referencePointAdded, this, [this](const Geometry::ReferencePoint&) {
        markMapModified();
    });

    // Path management
    connect(m_newPathBtn, &QPushButton::clicked, this, &MainWindow::createNewPath);
    connect(m_deletePathBtn, &QPushButton::clicked, this, &MainWindow::deletePath);
    connect(m_duplicatePathBtn, &QPushButton::clicked, this, &MainWindow::duplicatePath);
    connect(m_sendPathBtn, &QPushButton::clicked, this, &MainWindow::sendCurrentPathToRobot);
    connect(m_pathList, &QListWidget::currentRowChanged, this, &MainWindow::onPathSelectionChanged);

    // Robot connection
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::connectToRobot);
    connect(m_robotComm, &RobotComm::connected, this, &MainWindow::onRobotConnected);
    connect(m_robotComm, &RobotComm::disconnected, this, &MainWindow::onRobotDisconnected);
    connect(m_robotComm, &RobotComm::robotPoseUpdated, this, &MainWindow::onRobotPoseUpdated);

    // Robot shape
    connect(m_robotShapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onRobotShapeChanged);
    connect(m_robotWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRobotSizeChanged);
    connect(m_robotLengthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRobotSizeChanged);

    // Settings
    connect(m_gridSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onGridSizeChanged);

    // Waypoint editing
    connect(m_canvas, &MapCanvas::waypointDoubleClicked, this, &MainWindow::editWaypoint);

    // Update path list initially
    for (const auto& path : m_pathCollection.paths) {
        m_pathList->addItem(path.name);
    }
    if (!m_pathCollection.paths.isEmpty()) {
        m_pathList->setCurrentRow(0);
    }
}

void MainWindow::newMap() {
    // Check for unsaved changes
    if (m_mapModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save before creating a new map?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            saveMap();
            if (m_mapModified) {
                // User cancelled save dialog
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    m_mapData.clear();
    m_mapData.name = "New Map";
    m_currentMapFile.clear();
    m_mapModified = false;
    m_canvas->update();
    refreshWindowTitle();
    statusBar()->showMessage("New map created", 2000);
}

void MainWindow::openMap() {
    qDebug() << "openMap() called";

    // Check for unsaved changes
    if (m_mapModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "You have unsaved changes. Do you want to save before opening another map?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            saveMap();
            if (m_mapModified) {
                // User cancelled save dialog
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    qDebug() << "Opening file dialog from:" << dialogDirectory(kMapDirectoryKey);
    QString filename = QFileDialog::getOpenFileName(this, "Open Map",
                                                    dialogDirectory(kMapDirectoryKey),
                                                    "Map Files (*.json);;All Files (*)");
    qDebug() << "Selected file:" << filename;
    if (filename.isEmpty()) return;

    if (m_mapData.loadFromFile(filename)) {
        m_currentMapFile = filename;
        m_mapModified = false;
        m_canvas->update();
        m_gridSizeSpin->setValue(m_mapData.gridSize);

        setDialogDirectory(kMapDirectoryKey, filename);
        refreshWindowTitle();

        QFileInfo fileInfo(filename);
        statusBar()->showMessage(QString("Map loaded: %1").arg(fileInfo.fileName()), 3000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to load map file");
    }
}

void MainWindow::saveMap() {
    if (m_currentMapFile.isEmpty()) {
        saveMapAs();
    } else {
        if (m_mapData.saveToFile(m_currentMapFile)) {
            m_mapModified = false;
            setDialogDirectory(kMapDirectoryKey, m_currentMapFile);
            refreshWindowTitle();
            QFileInfo fileInfo(m_currentMapFile);
            statusBar()->showMessage(QString("Map saved: %1").arg(fileInfo.fileName()), 3000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to save map");
        }
    }
}

void MainWindow::saveMapAs() {
    qDebug() << "saveMapAs() called";

    QString initialDir = m_currentMapFile.isEmpty()
        ? dialogDirectory(kMapDirectoryKey)
        : QFileInfo(m_currentMapFile).absolutePath();
    qDebug() << "Opening save dialog from:" << initialDir;

    QString filename = QFileDialog::getSaveFileName(this, "Save Map As",
                                                    initialDir, "Map Files (*.json);;All Files (*)");
    qDebug() << "Selected file:" << filename;
    if (filename.isEmpty()) return;

    // Ensure .json extension
    if (!filename.endsWith(".json", Qt::CaseInsensitive)) {
        filename += ".json";
    }

    if (m_mapData.saveToFile(filename)) {
        m_currentMapFile = filename;
        m_mapModified = false;

        setDialogDirectory(kMapDirectoryKey, filename);
        refreshWindowTitle();

        QFileInfo fileInfo(filename);
        statusBar()->showMessage(QString("Map saved: %1").arg(fileInfo.fileName()), 3000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to save map");
    }
}

void MainWindow::exportMap() {
    saveMapAs();
}

void MainWindow::selectDrawLineTool() {
    m_canvas->setTool(MapCanvas::Tool::DrawLine);
    updateToolButtonSelection(m_drawLineToolAction);
    statusBar()->showMessage("Draw Line Tool: Click to start, drag and release to create a line");
}

void MainWindow::selectDrawPathTool() {
    m_canvas->setTool(MapCanvas::Tool::DrawPath);
    updateToolButtonSelection(m_drawPathToolAction);
    statusBar()->showMessage("Draw Path Tool: Click to add waypoints to the current path");
}

void MainWindow::selectSelectTool() {
    m_canvas->setTool(MapCanvas::Tool::Select);
    updateToolButtonSelection(m_selectToolAction);
    statusBar()->showMessage("Select Tool");
}

void MainWindow::selectMeasureTool() {
    m_canvas->setTool(MapCanvas::Tool::Measure);
    updateToolButtonSelection(m_measureToolAction);
    statusBar()->showMessage("Measure Tool: Click and drag to measure distance");
}

void MainWindow::selectPanTool() {
    m_canvas->setTool(MapCanvas::Tool::Pan);
    updateToolButtonSelection(m_panToolAction);
    statusBar()->showMessage("Pan Tool: Click and drag to pan the view");
}

void MainWindow::selectAddReferenceTool() {
    m_canvas->setTool(MapCanvas::Tool::AddReference);
    updateToolButtonSelection(m_addReferenceToolAction);
    statusBar()->showMessage("Add Reference: Click to place reference points");
}

void MainWindow::zoomIn() { m_canvas->zoomIn(); }
void MainWindow::zoomOut() { m_canvas->zoomOut(); }
void MainWindow::resetView() { m_canvas->resetView(); }
void MainWindow::fitToView() { m_canvas->fitToView(); }

void MainWindow::connectToRobot() {
    if (m_robotComm->isConnected()) {
        disconnectFromRobot();
        return;
    }

    QString ip = m_robotIpEdit->text();
    if (m_robotComm->connectToRobot(ip)) {
        // Connection successful will be handled by signal
    } else {
        QMessageBox::warning(this, "Connection Failed",
                           "Failed to connect to robot at " + ip);
    }
}

void MainWindow::disconnectFromRobot() {
    m_robotComm->disconnectFromRobot();
}

void MainWindow::sendCurrentPathToRobot() {
    if (!m_robotComm->isConnected()) {
        QMessageBox::warning(this, "Not Connected",
                           "Please connect to the robot first");
        return;
    }

    PathData* activePath = m_pathCollection.getActivePath();
    if (!activePath || activePath->waypoints.isEmpty()) {
        QMessageBox::warning(this, "No Path",
                           "Please create a path with waypoints first");
        return;
    }

    if (m_robotComm->sendPath(*activePath)) {
        statusBar()->showMessage("Path sent to robot", 3000);
    } else {
        QMessageBox::warning(this, "Send Failed", "Failed to send path to robot");
    }
}

void MainWindow::sendMapToRobot() {
    if (!m_robotComm->isConnected()) {
        QMessageBox::warning(this, "Not Connected",
                           "Please connect to the robot first");
        return;
    }

    if (m_mapData.lines.isEmpty()) {
        QMessageBox::warning(this, "No Map Data",
                           "Please draw lines on the map first");
        return;
    }

    if (m_robotComm->sendMapData(m_mapData)) {
        int lineCount = m_mapData.lines.size();
        int refPointCount = m_mapData.referencePoints.size();
        statusBar()->showMessage(QString("Map sent to robot: %1 lines, %2 reference points")
            .arg(lineCount).arg(refPointCount), 3000);
    } else {
        QMessageBox::warning(this, "Send Failed", "Failed to send map to robot");
    }
}

void MainWindow::sendReferencePointsToRobot() {
    if (!m_robotComm->isConnected()) {
        QMessageBox::warning(this, "Not Connected",
                           "Please connect to the robot first");
        return;
    }

    if (m_mapData.referencePoints.isEmpty()) {
        QMessageBox::warning(this, "No Reference Points",
                           "Please add reference points to the map first");
        return;
    }

    if (m_robotComm->sendReferencePoints(m_mapData.referencePoints)) {
        int refPointCount = m_mapData.referencePoints.size();
        statusBar()->showMessage(QString("Sent %1 reference points to robot").arg(refPointCount), 3000);
    } else {
        QMessageBox::warning(this, "Send Failed", "Failed to send reference points to robot");
    }
}

void MainWindow::onRobotConnected() {
    m_connectionStatusLabel->setText("Connected");
    m_connectionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
    m_connectBtn->setText("Disconnect");
    updateRobotStatus();
    statusBar()->showMessage("Connected to robot", 3000);
}

void MainWindow::onRobotDisconnected() {
    m_connectionStatusLabel->setText("Disconnected");
    m_connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    m_connectBtn->setText("Connect");
    updateRobotStatus();
    statusBar()->showMessage("Disconnected from robot", 3000);
}

void MainWindow::onRobotPoseUpdated(const Geometry::RobotPose& pose) {
    m_canvas->setRobotPose(pose);

    m_robotPositionLabel->setText(QString("Position: (%1, %2)")
                                 .arg(pose.position.x, 0, 'f', 3)
                                 .arg(pose.position.y, 0, 'f', 3));

    double headingDeg = pose.heading * 180.0 / M_PI;
    m_robotHeadingLabel->setText(QString("Heading: %1°")
                                .arg(headingDeg, 0, 'f', 1));

    // Calculate distance to nearest wall
    if (!m_mapData.lines.isEmpty()) {
        double distToWall = m_mapData.distanceToNearestWall(pose.position);
        m_robotStatusLabel->setText(QString("Robot: Connected | Dist to wall: %1 m")
                                   .arg(distToWall, 0, 'f', 3));
    }
}

void MainWindow::createNewPath() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Path",
                                        "Path name:", QLineEdit::Normal,
                                        QString("Path %1").arg(m_pathCollection.paths.size() + 1),
                                        &ok);
    if (ok && !name.isEmpty()) {
        PathData newPath(name);
        m_pathCollection.addPath(newPath);
        m_pathList->addItem(name);
        m_pathList->setCurrentRow(m_pathList->count() - 1);
    }
}

void MainWindow::deletePath() {
    int currentRow = m_pathList->currentRow();
    if (currentRow >= 0) {
        m_pathCollection.removePath(currentRow);
        delete m_pathList->takeItem(currentRow);
        m_canvas->update();
    }
}

void MainWindow::duplicatePath() {
    PathData* activePath = m_pathCollection.getActivePath();
    if (activePath) {
        PathData duplicate = *activePath;
        duplicate.name += " (Copy)";
        m_pathCollection.addPath(duplicate);
        m_pathList->addItem(duplicate.name);
    }
}

void MainWindow::onPathSelectionChanged() {
    m_pathCollection.activePathIndex = m_pathList->currentRow();

    PathData* activePath = m_pathCollection.getActivePath();
    if (activePath) {
        double length = activePath->totalLength();
        m_pathLengthLabel->setText(QString("Length: %1 m").arg(length, 0, 'f', 3));
    } else {
        m_pathLengthLabel->setText("Length: 0.0 m");
    }

    m_canvas->update();
}

void MainWindow::saveAllPaths() {
    QString filename = QFileDialog::getSaveFileName(this, "Save Paths",
                                                    dialogDirectory(kPathsDirectoryKey),
                                                    "Path Files (*.json)");
    if (filename.isEmpty()) {
        return;
    }

    if (!filename.endsWith(".json", Qt::CaseInsensitive)) {
        filename += ".json";
    }

    if (m_pathCollection.saveToFile(filename)) {
        setDialogDirectory(kPathsDirectoryKey, filename);
        QFileInfo info(filename);
        statusBar()->showMessage(QString("Paths saved: %1").arg(info.fileName()), 3000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to save paths");
    }
}

void MainWindow::loadPaths() {
    QString filename = QFileDialog::getOpenFileName(this, "Load Paths",
                                                    dialogDirectory(kPathsDirectoryKey),
                                                    "Path Files (*.json)");
    if (filename.isEmpty()) return;

    if (m_pathCollection.loadFromFile(filename)) {
        // Update path list
        m_pathList->clear();
        for (const auto& path : m_pathCollection.paths) {
            m_pathList->addItem(path.name);
        }

        if (m_pathCollection.activePathIndex >= 0 &&
            m_pathCollection.activePathIndex < m_pathList->count()) {
            m_pathList->setCurrentRow(m_pathCollection.activePathIndex);
        }

        m_canvas->update();
        setDialogDirectory(kPathsDirectoryKey, filename);
        QFileInfo info(filename);
        statusBar()->showMessage(QString("Paths loaded: %1").arg(info.fileName()), 3000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to load paths");
    }
}

void MainWindow::editWaypoint(int pathIndex, int waypointIndex) {
    if (pathIndex < 0 || pathIndex >= m_pathCollection.paths.size()) return;

    PathData& path = m_pathCollection.paths[pathIndex];
    if (waypointIndex < 0 || waypointIndex >= path.waypoints.size()) return;

    Geometry::Waypoint& waypoint = path.waypoints[waypointIndex];

    // Open waypoint dialog
    WaypointDialog dialog(waypoint, this);
    if (dialog.exec() == QDialog::Accepted) {
        waypoint = dialog.getWaypoint();
        m_canvas->update();

        // Update path length display if this is the active path
        if (pathIndex == m_pathCollection.activePathIndex) {
            double length = path.totalLength();
            m_pathLengthLabel->setText(QString("Length: %1 m").arg(length, 0, 'f', 3));
        }

        statusBar()->showMessage(QString("Waypoint updated: X=%1 Y=%2 θ=%3°")
            .arg(waypoint.position.x, 0, 'f', 3)
            .arg(waypoint.position.y, 0, 'f', 3)
            .arg(waypoint.heading * 180.0 / M_PI, 0, 'f', 1), 3000);
    }
}

void MainWindow::onRobotShapeChanged(int index) {
    Geometry::RobotPose pose = m_canvas->getCurrentPose();

    switch (index) {
        case 0: pose.shape = Geometry::RobotShape::Rectangle; break;
        case 1: pose.shape = Geometry::RobotShape::Square; break;
        case 2: pose.shape = Geometry::RobotShape::Triangle; break;
    }

    m_canvas->setRobotPose(pose);

    if (m_robotComm->isConnected()) {
        m_robotComm->sendRobotShape(pose.shape);
    }
}

void MainWindow::onRobotSizeChanged() {
    Geometry::RobotPose pose = m_canvas->getCurrentPose();
    pose.width = m_robotWidthSpin->value();
    pose.length = m_robotLengthSpin->value();
    m_canvas->setRobotPose(pose);
}

// Origin is now fixed at (0,0) as the global coordinate reference

void MainWindow::onGridSizeChanged(double size) {
    m_mapData.gridSize = size;
    m_canvas->update();
    markMapModified();
}

void MainWindow::updateStatusMessage(const QString& message) {
    m_coordsLabel->setText(message);
}

void MainWindow::updateRobotStatus() {
    if (m_robotComm->isConnected()) {
        m_robotStatusLabel->setText("Robot: Connected");
    } else {
        m_robotStatusLabel->setText("Robot: Disconnected");
    }
}
