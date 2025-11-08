#include "RobotComm.h"
#include "MapData.h"
#include <QJsonDocument>
#include <QJsonArray>

RobotComm::RobotComm(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_updateTimer(new QTimer(this))
    , m_isMoving(false)
{
    connect(m_socket, &QTcpSocket::connected, this, &RobotComm::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &RobotComm::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &RobotComm::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &RobotComm::onError);

    // Request robot state periodically (20Hz)
    m_updateTimer->setInterval(50);
    connect(m_updateTimer, &QTimer::timeout, this, &RobotComm::requestRobotState);
}

RobotComm::~RobotComm() {
    disconnectFromRobot();
}

bool RobotComm::connectToRobot(const QString& ipAddress, quint16 port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    // Non-blocking connection - status will be reported via signals
    m_socket->connectToHost(ipAddress, port);
    // Return true if connection attempt started, actual connection status
    // will be reported via connected() or connectionError() signals
    return (m_socket->state() == QAbstractSocket::ConnectingState ||
            m_socket->state() == QAbstractSocket::ConnectedState);
}

void RobotComm::disconnectFromRobot() {
    m_updateTimer->stop();
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool RobotComm::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool RobotComm::sendPath(const PathData& path) {
    if (!isConnected()) {
        return false;
    }

    QJsonObject message;
    message["type"] = "sendPath";
    message["path"] = path.toJson();

    sendJson(message);
    return true;
}

bool RobotComm::sendMapData(const MapData& mapData) {
    if (!isConnected()) {
        return false;
    }

    QJsonObject message;
    message["type"] = "sendMapData";
    message["mapData"] = mapData.toJson();

    sendJson(message);
    return true;
}

bool RobotComm::sendReferencePoints(const QVector<Geometry::ReferencePoint>& refPoints) {
    if (!isConnected()) {
        return false;
    }

    QJsonObject message;
    message["type"] = "sendReferencePoints";

    QJsonArray refPointsArray;
    for (const auto& rp : refPoints) {
        QJsonObject rpJson;
        rpJson["name"] = rp.name;
        rpJson["x"] = rp.position.x;
        rpJson["y"] = rp.position.y;
        if (rp.hasHeading) {
            rpJson["heading"] = rp.heading;
            rpJson["heading_deg"] = rp.heading * 180.0 / M_PI;
            rpJson["hasHeading"] = true;
        } else {
            rpJson["hasHeading"] = false;
        }
        refPointsArray.append(rpJson);
    }
    message["referencePoints"] = refPointsArray;

    sendJson(message);
    return true;
}

bool RobotComm::sendRobotShape(Geometry::RobotShape shape) {
    if (!isConnected()) {
        return false;
    }

    QJsonObject message;
    message["type"] = "setRobotShape";

    QString shapeStr;
    switch (shape) {
        case Geometry::RobotShape::Rectangle: shapeStr = "rectangle"; break;
        case Geometry::RobotShape::Square: shapeStr = "square"; break;
        case Geometry::RobotShape::Triangle: shapeStr = "triangle"; break;
    }
    message["shape"] = shapeStr;

    sendJson(message);
    return true;
}

bool RobotComm::sendCommand(const QString& command, const QJsonObject& data) {
    if (!isConnected()) {
        return false;
    }

    QJsonObject message;
    message["type"] = command;
    message["data"] = data;

    sendJson(message);
    return true;
}

void RobotComm::onConnected() {
    m_updateTimer->start();
    emit connected();
}

void RobotComm::onDisconnected() {
    m_updateTimer->stop();
    emit disconnected();
}

void RobotComm::onReadyRead() {
    m_receiveBuffer.append(m_socket->readAll());

    // Prevent unbounded buffer growth (max 1MB)
    const int MAX_BUFFER_SIZE = 1024 * 1024;
    if (m_receiveBuffer.size() > MAX_BUFFER_SIZE) {
        emit connectionError("Buffer overflow: received data exceeds 1MB without newline");
        disconnectFromRobot();
        m_receiveBuffer.clear();
        return;
    }

    // Process complete messages (assuming newline-delimited JSON)
    while (m_receiveBuffer.contains('\n')) {
        int newlineIndex = m_receiveBuffer.indexOf('\n');
        QByteArray messageData = m_receiveBuffer.left(newlineIndex);
        m_receiveBuffer.remove(0, newlineIndex + 1);

        if (!messageData.isEmpty()) {
            parseIncomingData(messageData);
        }
    }
}

void RobotComm::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit connectionError(m_socket->errorString());
}

void RobotComm::requestRobotState() {
    if (!isConnected()) {
        return;
    }

    QJsonObject message;
    message["type"] = "getState";
    sendJson(message);
}

void RobotComm::parseIncomingData(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return;
    }

    QJsonObject json = doc.object();
    QString type = json["type"].toString();

    if (type == "robotPose") {
        // Update robot pose
        m_currentPose.position.x = json["x"].toDouble();
        m_currentPose.position.y = json["y"].toDouble();
        m_currentPose.heading = json["heading"].toDouble();

        emit robotPoseUpdated(m_currentPose);
    }
    else if (type == "status") {
        QString status = json["status"].toString();
        m_isMoving = json["moving"].toBool();

        emit robotStatusUpdated(status);
    }
    else if (type == "pathExecutionStarted") {
        emit pathExecutionStarted();
    }
    else if (type == "pathExecutionFinished") {
        bool success = json["success"].toBool();
        emit pathExecutionFinished(success);
    }
}

void RobotComm::sendJson(const QJsonObject& json) {
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n'); // Message delimiter
    m_socket->write(data);
    m_socket->flush();
}
