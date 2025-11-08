#include "WaypointDialog.h"
#include <QVBoxLayout>
#include <cmath>

WaypointDialog::WaypointDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Edit Waypoint");
    setupUI();
}

WaypointDialog::WaypointDialog(const Geometry::Waypoint& waypoint, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Edit Waypoint");
    setupUI();
    setWaypoint(waypoint);
}

void WaypointDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout;

    // X coordinate
    m_xSpin = new QDoubleSpinBox;
    m_xSpin->setRange(-100.0, 100.0);
    m_xSpin->setValue(0.0);
    m_xSpin->setSuffix(" m");
    m_xSpin->setDecimals(3);
    m_xSpin->setSingleStep(0.1);
    formLayout->addRow("X Position:", m_xSpin);

    // Y coordinate
    m_ySpin = new QDoubleSpinBox;
    m_ySpin->setRange(-100.0, 100.0);
    m_ySpin->setValue(0.0);
    m_ySpin->setSuffix(" m");
    m_ySpin->setDecimals(3);
    m_ySpin->setSingleStep(0.1);
    formLayout->addRow("Y Position:", m_ySpin);

    // Theta (heading angle)
    m_thetaSpin = new QDoubleSpinBox;
    m_thetaSpin->setRange(-180.0, 180.0);
    m_thetaSpin->setValue(0.0);
    m_thetaSpin->setSuffix(" °");
    m_thetaSpin->setDecimals(1);
    m_thetaSpin->setSingleStep(5.0);
    m_thetaSpin->setWrapping(true); // Allow wrapping from 180 to -180
    formLayout->addRow("Theta (θ):", m_thetaSpin);

    // Angle preview
    m_anglePreviewLabel = new QLabel("→ (0° = Right, 90° = Up)");
    m_anglePreviewLabel->setStyleSheet("color: gray; font-style: italic;");
    formLayout->addRow("", m_anglePreviewLabel);

    // Velocity
    m_velocitySpin = new QDoubleSpinBox;
    m_velocitySpin->setRange(0.0, 10.0);
    m_velocitySpin->setValue(1.0);
    m_velocitySpin->setSuffix(" m/s");
    m_velocitySpin->setDecimals(2);
    m_velocitySpin->setSingleStep(0.1);
    formLayout->addRow("Velocity:", m_velocitySpin);

    mainLayout->addLayout(formLayout);

    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_thetaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &WaypointDialog::updateAnglePreview);

    updateAnglePreview();
}

void WaypointDialog::updateAnglePreview() {
    double degrees = m_thetaSpin->value();
    QString arrow;

    // Visual arrow based on angle
    if (degrees >= -22.5 && degrees < 22.5) {
        arrow = "→"; // Right
    } else if (degrees >= 22.5 && degrees < 67.5) {
        arrow = "↗"; // Up-right
    } else if (degrees >= 67.5 && degrees < 112.5) {
        arrow = "↑"; // Up
    } else if (degrees >= 112.5 && degrees < 157.5) {
        arrow = "↖"; // Up-left
    } else if (degrees >= 157.5 || degrees < -157.5) {
        arrow = "←"; // Left
    } else if (degrees >= -157.5 && degrees < -112.5) {
        arrow = "↙"; // Down-left
    } else if (degrees >= -112.5 && degrees < -67.5) {
        arrow = "↓"; // Down
    } else {
        arrow = "↘"; // Down-right
    }

    m_anglePreviewLabel->setText(QString("%1 (0° = Right, 90° = Up, -90° = Down)").arg(arrow));
}

Geometry::Waypoint WaypointDialog::getWaypoint() const {
    Geometry::Point pos(m_xSpin->value(), m_ySpin->value());
    double thetaRadians = m_thetaSpin->value() * M_PI / 180.0;
    double velocity = m_velocitySpin->value();

    return Geometry::Waypoint(pos, thetaRadians, velocity);
}

void WaypointDialog::setWaypoint(const Geometry::Waypoint& waypoint) {
    m_xSpin->setValue(waypoint.position.x);
    m_ySpin->setValue(waypoint.position.y);
    m_thetaSpin->setValue(waypoint.heading * 180.0 / M_PI);
    m_velocitySpin->setValue(waypoint.velocity);
    updateAnglePreview();
}
