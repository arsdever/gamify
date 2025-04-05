#include <QCursor>
#include <QEnterEvent>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleHints>

#include <common/logging.hpp>

#include "spinbox.hpp"

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

namespace ui
{
struct SpinBox::SpinBoxPrivate
{
    bool _isValueDragging = false;
    double _value = 0.0;
    double _valueBeforeDrag = 0.0;
    bool _mouseOnControl = false;

    double _min = 0.0;
    double _max = 100.0;
    double _singleStep = 1.0;
    double _step = 0.1;
    double _pageStep = 10.0;
    int _decimals = 2;
    bool _accelerated = false;

    QString _label;

    Qt::MouseButton _mouseState = Qt::MouseButton::NoButton;
    QPoint _oldClickPos;
};

SpinBox::SpinBox(QWidget* parent)
    : QWidget(parent)
    , _p(std::make_unique<SpinBoxPrivate>())
{
    setMinimumSize(30, 20);
    resize(100, 20);
}

SpinBox::~SpinBox() = default;

void SpinBox::setLabel(const QString& label) { _p->_label = label; }

QString SpinBox::label() const { return _p->_label; }

void SpinBox::setValue(double value)
{
    value = CLAMP(value, _p->_min, _p->_max);
    if (_p->_value != value)
    {
        _p->_value = value;
        valueChanged(value);
        update();
    }
}

double SpinBox::value() const { return _p->_value; }

void SpinBox::setRange(double min, double max)
{
    _p->_min = min;
    _p->_max = max;
}

double SpinBox::min() const { return _p->_min; }

double SpinBox::max() const { return _p->_max; }

void SpinBox::setSingleStep(double step) { _p->_singleStep = step; }

double SpinBox::singleStep() const { return _p->_singleStep; }

void SpinBox::setStep(double step) { _p->_step = step; }

double SpinBox::step() const { return _p->_step; }

void SpinBox::setPageStep(double step) { _p->_pageStep = step; }

double SpinBox::pageStep() const { return _p->_pageStep; }

void SpinBox::setDecimals(int decimals) { _p->_decimals = decimals; }

int SpinBox::decimals() const { return _p->_decimals; }

void SpinBox::setAccelerated(bool accelerated)
{
    _p->_accelerated = accelerated;
}

bool SpinBox::isAccelerated() const { return _p->_accelerated; }

void SpinBox::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    event->accept();

    _p->_mouseOnControl = true;
    update();
    setCursor(Qt::CursorShape::SizeHorCursor);
}

void SpinBox::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    event->accept();

    _p->_mouseOnControl = false;
    update();
    setCursor(Qt::CursorShape::ArrowCursor);
}

void SpinBox::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    event->accept();

    _p->_mouseState = event->button();
    _p->_oldClickPos = event->pos();
}

void SpinBox::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    event->accept();

    _p->_mouseState = Qt::MouseButton::NoButton;
    _p->_isValueDragging = false;
}

void SpinBox::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    event->accept();

    auto diff = event->pos() - _p->_oldClickPos;

    auto stepSize = _p->_singleStep;
    if (qApp->queryKeyboardModifiers() & Qt::KeyboardModifier::ShiftModifier)
    {
        stepSize = _p->_pageStep;
    }
    else if (qApp->queryKeyboardModifiers() &
             Qt::KeyboardModifier::ControlModifier)
    {
        stepSize = _p->_step;
    }

    if (_p->_isValueDragging)
    {
        setValue(_p->_valueBeforeDrag + diff.x() * stepSize);
    }
    else if (_p->_mouseState == Qt::MouseButton::LeftButton)
    {
        if (diff.manhattanLength() > qApp->styleHints()->startDragDistance())
        {
            _p->_isValueDragging = true;
            _p->_valueBeforeDrag = value();
        }
    }
}

void SpinBox::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QPalette pal = palette();

    painter.setPen(pal.color(QPalette::Button));
    painter.setBrush(
        pal.color(_p->_mouseOnControl ? QPalette::Highlight : QPalette::Base));
    painter.drawRoundedRect(
        QRectF(rect()).adjusted(0.5f, 0.5f, -0.5f, -0.5f), 5.0f, 5.0f);

    painter.setPen(pal.color(QPalette::Text));
    painter.drawText(QRectF(rect()).adjusted(
                         15.5f + fontMetrics().boundingRect(label()).width(),
                         0.5f,
                         -15.5f,
                         -0.5f),
                     Qt::AlignRight,
                     QString::number(value(), 'f', decimals()));
    painter.drawText(QRectF(rect()).adjusted(15.5f, 0.5f, -15.5f, -0.5f),
                     Qt::AlignLeft,
                     label());

    event->accept();
}

void SpinBox::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
    event->accept();

    if (event->key() == Qt::Key_Up)
    {
        setValue(value() + singleStep());
    }
    else if (event->key() == Qt::Key_Down)
    {
        setValue(value() - singleStep());
    }
    else if (event->key() == Qt::Key_Left)
    {
        setValue(value() - step());
    }
    else if (event->key() == Qt::Key_Right)
    {
        setValue(value() + step());
    }
    else if (event->key() == Qt::Key_PageDown)
    {
        setValue(value() - pageStep());
    }
    else if (event->key() == Qt::Key_PageUp)
    {
        setValue(value() + pageStep());
    }
}

void SpinBox::wheelEvent(QWheelEvent* event)
{
    event->accept();

    auto magX = event->angleDelta().x();
    auto magY = event->angleDelta().y();

    setValue(value() + magY * step());
    setValue(value() + magX * singleStep());
}
} // namespace ui
