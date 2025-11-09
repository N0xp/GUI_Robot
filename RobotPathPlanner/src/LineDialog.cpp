#include "LineDialog.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QtMath>

namespace {
constexpr double kDefaultMin = -100.0;
constexpr double kDefaultMax = 100.0;
}

LineDialog::LineDialog(const Geometry::Line& line, QWidget* parent)
    : QDialog(parent)
    , m_startXSpin(new QDoubleSpinBox(this))
    , m_startYSpin(new QDoubleSpinBox(this))
    , m_lengthSpin(new QDoubleSpinBox(this))
    , m_angleSpin(new QDoubleSpinBox(this))
    , m_endLabel(new QLabel(this)) {

    setWindowTitle("Edit Line");
    setModal(true);
    resize(360, 0);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(20, 25, 35));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Base, QColor(15, 20, 30));
    pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::ButtonText, Qt::white);
    pal.setColor(QPalette::Button, QColor(31, 43, 61));
    setPalette(pal);

    setStyleSheet(
        "QLabel { color: white; }"
        "QDoubleSpinBox { color: white; background: #101622; border: 1px solid #3c4f6b; padding: 4px; }"
        "QDialogButtonBox QPushButton { color: white; background-color: #1f2b3d; padding: 4px 12px; border: 1px solid #3c4f6b; }"
        "QDialogButtonBox QPushButton:hover { background-color: #27354a; }"
    );

    m_startXSpin->setRange(kDefaultMin, kDefaultMax);
    m_startXSpin->setDecimals(3);
    m_startXSpin->setSingleStep(0.1);
    m_startXSpin->setValue(line.start.x);
    m_startXSpin->setSuffix(" m");

    m_startYSpin->setRange(kDefaultMin, kDefaultMax);
    m_startYSpin->setDecimals(3);
    m_startYSpin->setSingleStep(0.1);
    m_startYSpin->setValue(line.start.y);
    m_startYSpin->setSuffix(" m");

    m_lengthSpin->setRange(0.01, 200.0);
    m_lengthSpin->setDecimals(3);
    m_lengthSpin->setSingleStep(0.1);
    m_lengthSpin->setValue(line.length());
    m_lengthSpin->setSuffix(" m");

    m_angleSpin->setRange(-360.0, 360.0);
    m_angleSpin->setDecimals(1);
    m_angleSpin->setSingleStep(1.0);
    m_angleSpin->setValue(line.angleDegrees());
    m_angleSpin->setSuffix(" °");

    m_endLabel->setText("End: ");

    auto* formLayout = new QFormLayout;
    formLayout->addRow("Start X:", m_startXSpin);
    formLayout->addRow("Start Y:", m_startYSpin);
    formLayout->addRow("Length:", m_lengthSpin);
    formLayout->addRow("Angle:", m_angleSpin);
    formLayout->addRow("End Preview:", m_endLabel);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LineDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LineDialog::reject);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    auto connectSpin = [this](QDoubleSpinBox* spin) {
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double) { updatePreview(); });
    };
    connectSpin(m_startXSpin);
    connectSpin(m_startYSpin);
    connectSpin(m_lengthSpin);
    connectSpin(m_angleSpin);

    updatePreview();
}

Geometry::Line LineDialog::line() const {
    Geometry::Line result;
    result.start.x = m_startXSpin->value();
    result.start.y = m_startYSpin->value();

    double length = m_lengthSpin->value();
    double angleRadians = qDegreesToRadians(m_angleSpin->value());

    result.end.x = result.start.x + std::cos(angleRadians) * length;
    result.end.y = result.start.y + std::sin(angleRadians) * length;

    return result;
}

void LineDialog::updatePreview() {
    Geometry::Line currentLine = line();
    m_endLabel->setText(
        QString("End: X %1 m, Y %2 m")
            .arg(currentLine.end.x, 0, 'f', 3)
            .arg(currentLine.end.y, 0, 'f', 3));
}
