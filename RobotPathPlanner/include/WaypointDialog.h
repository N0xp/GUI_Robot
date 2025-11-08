#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include "Geometry.h"

class WaypointDialog : public QDialog {
    Q_OBJECT

public:
    explicit WaypointDialog(QWidget* parent = nullptr);
    explicit WaypointDialog(const Geometry::Waypoint& waypoint, QWidget* parent = nullptr);

    Geometry::Waypoint getWaypoint() const;
    void setWaypoint(const Geometry::Waypoint& waypoint);

private:
    void setupUI();
    void updateAnglePreview();

    QDoubleSpinBox* m_xSpin;
    QDoubleSpinBox* m_ySpin;
    QDoubleSpinBox* m_thetaSpin;      // Angle in degrees
    QDoubleSpinBox* m_velocitySpin;
    QLabel* m_anglePreviewLabel;
    QDialogButtonBox* m_buttonBox;
};

#endif // WAYPOINTDIALOG_H
