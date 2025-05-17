#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>

#include "vec3_widget.hpp"

#include "spinbox.hpp"

namespace ui
{
struct Vec3Widget::Vec3WidgetPrivate
{
    glm::dvec3 _value { 0 };
    QLabel* _label { nullptr };
    SpinBox* _xSpinBox { nullptr };
    SpinBox* _ySpinBox { nullptr };
    SpinBox* _zSpinBox { nullptr };
};

Vec3Widget::Vec3Widget(QWidget* parent)
    : QWidget(parent)
{
    _p = std::make_unique<Vec3WidgetPrivate>();

    QWidget* spinBoxContainer = new QWidget(this);
    {
        auto layout = new QVBoxLayout(this);
        spinBoxContainer->setLayout(layout);

        setMinimumSize(100, 0);

        _p->_xSpinBox = new SpinBox(this);
        _p->_ySpinBox = new SpinBox(this);
        _p->_zSpinBox = new SpinBox(this);

        layout->addWidget(_p->_xSpinBox);
        layout->addWidget(_p->_ySpinBox);
        layout->addWidget(_p->_zSpinBox);
        layout->setSpacing(0);

        _p->_xSpinBox->setLabel("x");
        _p->_ySpinBox->setLabel("y");
        _p->_zSpinBox->setLabel("z");

        _p->_xSpinBox->setRange(-1000000, 1000000);
        _p->_ySpinBox->setRange(-1000000, 1000000);
        _p->_zSpinBox->setRange(-1000000, 1000000);

        _p->_xSpinBox->setDecimals(6);
        _p->_ySpinBox->setDecimals(6);
        _p->_zSpinBox->setDecimals(6);

        _p->_xSpinBox->setStep(0.01);
        _p->_ySpinBox->setStep(0.01);
        _p->_zSpinBox->setStep(0.01);

        _p->_xSpinBox->setSingleStep(0.1);
        _p->_ySpinBox->setSingleStep(0.1);
        _p->_zSpinBox->setSingleStep(0.1);

        _p->_xSpinBox->setPageStep(10);
        _p->_ySpinBox->setPageStep(10);
        _p->_zSpinBox->setPageStep(10);

        _p->_xSpinBox->setValue(0);
        _p->_ySpinBox->setValue(0);
        _p->_zSpinBox->setValue(0);

        connect(_p->_xSpinBox,
                &SpinBox::valueChanged,
                this,
                [ this ](double value)
        {
            _p->_value.x = value;
            emit valueChanged(_p->_value);
        });
        connect(_p->_ySpinBox,
                &SpinBox::valueChanged,
                this,
                [ this ](double value)
        {
            _p->_value.y = value;
            emit valueChanged(_p->_value);
        });
        connect(_p->_zSpinBox,
                &SpinBox::valueChanged,
                this,
                [ this ](double value)
        {
            _p->_value.z = value;
            emit valueChanged(_p->_value);
        });
    }

    setLayout(new QVBoxLayout(this));
    layout()->setSpacing(0);
    _p->_label = new QLabel(this);
    layout()->addWidget(_p->_label);
    layout()->addWidget(spinBoxContainer);
    layout()->setContentsMargins(QMargins(0, 0, 0, 0));
    layout()->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

Vec3Widget::~Vec3Widget() = default;

void Vec3Widget::setLabel(const QString& label) { _p->_label->setText(label); }

QString Vec3Widget::label() const { return _p->_label->text(); }

void Vec3Widget::setRangeMin(glm::dvec3 min)
{
    auto oldMax = max();
    _p->_xSpinBox->setRange(min.x, oldMax.x);
    _p->_ySpinBox->setRange(min.y, oldMax.y);
    _p->_zSpinBox->setRange(min.z, oldMax.z);
}

void Vec3Widget::setRangeMax(glm::dvec3 max)
{
    auto oldMin = min();
    _p->_xSpinBox->setRange(oldMin.x, max.x);
    _p->_ySpinBox->setRange(oldMin.y, max.y);
    _p->_zSpinBox->setRange(oldMin.z, max.z);
}

glm::dvec3 Vec3Widget::min() const
{
    return { _p->_xSpinBox->min(), _p->_ySpinBox->min(), _p->_zSpinBox->min() };
}

glm::dvec3 Vec3Widget::max() const
{
    return { _p->_xSpinBox->max(), _p->_ySpinBox->max(), _p->_zSpinBox->max() };
}

void Vec3Widget::setStep(glm::dvec3 step)
{
    _p->_xSpinBox->setStep(step.x);
    _p->_ySpinBox->setStep(step.y);
    _p->_zSpinBox->setStep(step.z);
}

glm::dvec3 Vec3Widget::step() const
{
    return { _p->_xSpinBox->step(),
             _p->_ySpinBox->step(),
             _p->_zSpinBox->step() };
}

void Vec3Widget::setSingleStep(glm::dvec3 step)
{
    _p->_xSpinBox->setSingleStep(step.x);
    _p->_ySpinBox->setSingleStep(step.y);
    _p->_zSpinBox->setSingleStep(step.z);
}

glm::dvec3 Vec3Widget::singleStep() const
{
    return { _p->_xSpinBox->singleStep(),
             _p->_ySpinBox->singleStep(),
             _p->_zSpinBox->singleStep() };
}

void Vec3Widget::setPageStep(glm::dvec3 step)
{
    _p->_xSpinBox->setPageStep(step.x);
    _p->_ySpinBox->setPageStep(step.y);
    _p->_zSpinBox->setPageStep(step.z);
}

glm::dvec3 Vec3Widget::pageStep() const
{
    return { _p->_xSpinBox->pageStep(),
             _p->_ySpinBox->pageStep(),
             _p->_zSpinBox->pageStep() };
}

void Vec3Widget::setDecimals(int decimals)
{
    _p->_xSpinBox->setDecimals(decimals);
    _p->_ySpinBox->setDecimals(decimals);
    _p->_zSpinBox->setDecimals(decimals);
}

int Vec3Widget::decimals() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    return _p->_xSpinBox->decimals();
}

void Vec3Widget::setAccelerated(bool accelerated)
{
    _p->_xSpinBox->setAccelerated(accelerated);
    _p->_ySpinBox->setAccelerated(accelerated);
    _p->_zSpinBox->setAccelerated(accelerated);
}

bool Vec3Widget::isAccelerated() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    // them.
    return _p->_xSpinBox->isAccelerated();
}

void Vec3Widget::setValue(glm::dvec3 value, bool force)
{
    if (force || (_p->_value != value))
    {
        _p->_value = value;
        bool xB = _p->_xSpinBox->blockSignals(true);
        bool yB = _p->_ySpinBox->blockSignals(true);
        bool zB = _p->_zSpinBox->blockSignals(true);
        _p->_xSpinBox->setValue(_p->_value.x);
        _p->_ySpinBox->setValue(_p->_value.y);
        _p->_zSpinBox->setValue(_p->_value.z);
        _p->_xSpinBox->blockSignals(xB);
        _p->_ySpinBox->blockSignals(yB);
        _p->_zSpinBox->blockSignals(zB);
        emit valueChanged(_p->_value);
    }
}

glm::dvec3 Vec3Widget::value() { return _p->_value; }
} // namespace ui
