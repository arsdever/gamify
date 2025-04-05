#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>

#include "vec3_widget.hpp"

#include "spinbox.hpp"

namespace ui
{
Vec3Widget::Vec3Widget(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    setLayout(layout);

    setMinimumSize(100, 0);

    _xSpinBox = new SpinBox(this);
    _ySpinBox = new SpinBox(this);
    _zSpinBox = new SpinBox(this);

    layout->addWidget(_xSpinBox);
    layout->addWidget(_ySpinBox);
    layout->addWidget(_zSpinBox);
    layout->setSpacing(0);

    _xSpinBox->setLabel("x");
    _ySpinBox->setLabel("y");
    _zSpinBox->setLabel("z");

    _xSpinBox->setRange(-1000000, 1000000);
    _ySpinBox->setRange(-1000000, 1000000);
    _zSpinBox->setRange(-1000000, 1000000);

    _xSpinBox->setDecimals(6);
    _ySpinBox->setDecimals(6);
    _zSpinBox->setDecimals(6);

    _xSpinBox->setStep(0.01);
    _ySpinBox->setStep(0.01);
    _zSpinBox->setStep(0.01);

    _xSpinBox->setSingleStep(0.1);
    _ySpinBox->setSingleStep(0.1);
    _zSpinBox->setSingleStep(0.1);

    _xSpinBox->setPageStep(10);
    _ySpinBox->setPageStep(10);
    _zSpinBox->setPageStep(10);

    _xSpinBox->setValue(0);
    _ySpinBox->setValue(0);
    _zSpinBox->setValue(0);

    connect(_xSpinBox,
            &SpinBox::valueChanged,
            this,
            [ this ](double value)
    {
        _value.x = value;
        emit valueChanged(_value);
    });
    connect(_ySpinBox,
            &SpinBox::valueChanged,
            this,
            [ this ](double value)
    {
        _value.y = value;
        emit valueChanged(_value);
    });
    connect(_zSpinBox,
            &SpinBox::valueChanged,
            this,
            [ this ](double value)
    {
        _value.z = value;
        emit valueChanged(_value);
    });
}

void Vec3Widget::setRangeMin(glm::vec3 min)
{
    auto oldMax = max();
    _xSpinBox->setRange(min.x, oldMax.x);
    _ySpinBox->setRange(min.y, oldMax.y);
    _zSpinBox->setRange(min.z, oldMax.z);
}

void Vec3Widget::setRangeMax(glm::vec3 max)
{
    auto oldMin = min();
    _xSpinBox->setRange(oldMin.x, max.x);
    _ySpinBox->setRange(oldMin.y, max.y);
    _zSpinBox->setRange(oldMin.z, max.z);
}

glm::vec3 Vec3Widget::min() const
{
    return { _xSpinBox->min(), _ySpinBox->min(), _zSpinBox->min() };
}

glm::vec3 Vec3Widget::max() const
{
    return { _xSpinBox->max(), _ySpinBox->max(), _zSpinBox->max() };
}

void Vec3Widget::setStep(glm::vec3 step)
{
    _xSpinBox->setStep(step.x);
    _ySpinBox->setStep(step.y);
    _zSpinBox->setStep(step.z);
}

glm::vec3 Vec3Widget::step() const
{
    return { _xSpinBox->step(), _ySpinBox->step(), _zSpinBox->step() };
}

void Vec3Widget::setSingleStep(glm::vec3 step)
{
    _xSpinBox->setSingleStep(step.x);
    _ySpinBox->setSingleStep(step.y);
    _zSpinBox->setSingleStep(step.z);
}

glm::vec3 Vec3Widget::singleStep() const
{
    return { _xSpinBox->singleStep(),
             _ySpinBox->singleStep(),
             _zSpinBox->singleStep() };
}

void Vec3Widget::setPageStep(glm::vec3 step)
{
    _xSpinBox->setPageStep(step.x);
    _ySpinBox->setPageStep(step.y);
    _zSpinBox->setPageStep(step.z);
}

glm::vec3 Vec3Widget::pageStep() const
{
    return { _xSpinBox->pageStep(),
             _ySpinBox->pageStep(),
             _zSpinBox->pageStep() };
}

void Vec3Widget::setDecimals(int decimals)
{
    _xSpinBox->setDecimals(decimals);
    _ySpinBox->setDecimals(decimals);
    _zSpinBox->setDecimals(decimals);
}

int Vec3Widget::decimals() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    return _xSpinBox->decimals();
}

void Vec3Widget::setAccelerated(bool accelerated)
{
    _xSpinBox->setAccelerated(accelerated);
    _ySpinBox->setAccelerated(accelerated);
    _zSpinBox->setAccelerated(accelerated);
}

bool Vec3Widget::isAccelerated() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    // them.
    return _xSpinBox->isAccelerated();
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
