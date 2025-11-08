#ifndef ROBOTCOMM_H
#define ROBOTCOMM_H

#include "Geometry.h"
#include "PathData.h"
#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QJsonObject>

// Simple NetworkTables-like communication using TCP/JSON
// For production, integrate actual NetworkTables C++ library

class RobotComm : public QObject {
    Q_OBJECT

public:
    explicit RobotComm(QObject* parent = nullptr);
    ~RobotComm();

    // Connection
    bool connectToRobot(const QString& ipAddress, quint16 port = 5800);
    void disconnectFromRobot();
    bool isConnected() const;

    // Send path to robot
    bool sendPath(const PathData& path);

    // Send map data to robot
    bool sendMapData(const class MapData& mapData);
    bool sendReferencePoints(const QVector<Geometry::ReferencePoint>& refPoints);

    // Send individual commands
    bool sendRobotShape(Geometry::RobotShape shape);
    bool sendCommand(const QString& command, const QJsonObject& data);

    // Get current robot state
    Geometry::RobotPose getCurrentPose() const { return m_currentPose; }
    bool isRobotMoving() const { return m_isMoving; }

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void robotPoseUpdated(const Geometry::RobotPose& pose);
    void robotStatusUpdated(const QString& status);
    void pathExecutionStarted();
    void pathExecutionFinished(bool success);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void requestRobotState();

private:
    void parseIncomingData(const QByteArray& data);
    void sendJson(const QJsonObject& json);

    QTcpSocket* m_socket;
    QTimer* m_updateTimer;
    Geometry::RobotPose m_currentPose;
    bool m_isMoving;
    QByteArray m_receiveBuffer;
};

#endif // ROBOTCOMM_H
