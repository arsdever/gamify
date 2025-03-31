#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>

#include "vec3_widget.hpp"

namespace ui
{
Vec3Widget::Vec3Widget(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    setLayout(layout);

    _xSpinBox = new QDoubleSpinBox(this);
    _ySpinBox = new QDoubleSpinBox(this);
    _zSpinBox = new QDoubleSpinBox(this);
    layout->addWidget(new QLabel("x", this));
    layout->addWidget(_xSpinBox);
    layout->addWidget(new QLabel("y", this));
    layout->addWidget(_ySpinBox);
    layout->addWidget(new QLabel("z", this));
    layout->addWidget(_zSpinBox);

    _xSpinBox->setRange(-1000000, 1000000);
    _ySpinBox->setRange(-1000000, 1000000);
    _zSpinBox->setRange(-1000000, 1000000);

    _xSpinBox->setDecimals(6);
    _ySpinBox->setDecimals(6);
    _zSpinBox->setDecimals(6);

    _xSpinBox->setSingleStep(0.01);
    _ySpinBox->setSingleStep(0.01);
    _zSpinBox->setSingleStep(0.01);

    _xSpinBox->setValue(0);
    _ySpinBox->setValue(0);
    _zSpinBox->setValue(0);

    _xSpinBox->setKeyboardTracking(false);
    _ySpinBox->setKeyboardTracking(false);
    _zSpinBox->setKeyboardTracking(false);

    connect(_xSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [ this ](double value)
    {
        _value.x = value;
        emit valueChanged(_value);
    });
    connect(_ySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [ this ](double value)
    {
        _value.y = value;
        emit valueChanged(_value);
    });
    connect(_zSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [ this ](double value)
    {
        _value.z = value;
        emit valueChanged(_value);
    });
}

void Vec3Widget::setValue(glm::vec3 value, bool force)
{
    if (force || (_value != value))
    {
        _value = value;
        bool xB = _xSpinBox->blockSignals(true);
        bool yB = _ySpinBox->blockSignals(true);
        bool zB = _zSpinBox->blockSignals(true);
        _xSpinBox->setValue(_value.x);
        _ySpinBox->setValue(_value.y);
        _zSpinBox->setValue(_value.z);
        _xSpinBox->blockSignals(xB);
        _ySpinBox->blockSignals(yB);
        _zSpinBox->blockSignals(zB);
        emit valueChanged(_value);
    }
}

glm::vec3 Vec3Widget::value() { return _value; }
} // namespace ui
