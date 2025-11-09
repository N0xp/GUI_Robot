#pragma once

#include <QDialog>
#include "Geometry.h"

class QDoubleSpinBox;
class QLabel;

class LineDialog : public QDialog {
    Q_OBJECT

public:
    explicit LineDialog(const Geometry::Line& line, QWidget* parent = nullptr);

    Geometry::Line line() const;

private slots:
    void updatePreview();

private:
    QDoubleSpinBox* m_startXSpin;
    QDoubleSpinBox* m_startYSpin;
    QDoubleSpinBox* m_lengthSpin;
    QDoubleSpinBox* m_angleSpin;
    QLabel* m_endLabel;
};
