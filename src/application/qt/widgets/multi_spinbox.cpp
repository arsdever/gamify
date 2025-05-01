#include <QLabel>
#include <QVBoxLayout>

#include "multi_spinbox.hpp"

#include "spinbox.hpp"

namespace ui
{
template <size_t N, typename T>
struct MultiSpinBox<N, T>::impl
{
    glm::vec<N, T> _value { 0 };
    QLabel* _label { nullptr };
    std::array<SpinBox*, N> _spinBoxArray { nullptr };
};

template <size_t N, typename T>
MultiSpinBox<N, T>::MultiSpinBox(QWidget* parent)
    : QWidget(parent)
{
    _p = std::make_unique<impl>();

    QWidget* spinBoxContainer = new QWidget(this);
    {
        auto layout = new QVBoxLayout(this);
        spinBoxContainer->setLayout(layout);

        setMinimumSize(100, 0);

        static_assert(N <= 4, "MultiSpinBox only supports up to 4 components.");
        static constexpr std::array<const char*, 4> labels {
            "x",
            "y",
            "z",
            "w",
        };

        for (size_t i = 0; i < N; ++i)
        {
            _p->_spinBoxArray[ i ] = new SpinBox(this);
            _p->_spinBoxArray[ i ]->setLabel(labels[ i ]);
            _p->_spinBoxArray[ i ]->setRange(-1000000, 1000000);
            _p->_spinBoxArray[ i ]->setDecimals(6);
            _p->_spinBoxArray[ i ]->setStep(0.01);
            _p->_spinBoxArray[ i ]->setSingleStep(0.1);
            _p->_spinBoxArray[ i ]->setPageStep(10);
            _p->_spinBoxArray[ i ]->setValue(0);

            connect(_p->_spinBoxArray[ i ],
                    &SpinBox::valueChanged,
                    this,
                    [ this, i ](double value)
            {
                _p->_value[ i ] = value;
                value_changed(_p->_value);
            });

            layout->addWidget(_p->_spinBoxArray[ i ]);
        }

        layout->setSpacing(0);
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

template <size_t N, typename T>
MultiSpinBox<N, T>::~MultiSpinBox() = default;

template <size_t N, typename T>
void MultiSpinBox<N, T>::setLabel(const QString& label)
{
    _p->_label->setText(label);
}

template <size_t N, typename T>
QString MultiSpinBox<N, T>::label() const
{
    return _p->_label->text();
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setRangeMin(glm::vec<N, T> min)
{
    auto oldMax = max();
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setRange(min[ i ], oldMax[ i ]);
    }
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setRangeMax(glm::vec<N, T> max)
{
    auto oldMin = min();
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setRange(oldMin[ i ], max[ i ]);
    }
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::min() const
{
    glm::vec<N, T> value {};
    for (size_t i = 0; i < N; ++i)
    {
        value[ i ] = _p->_spinBoxArray[ i ]->min();
    }
    return value;
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::max() const
{
    glm::vec<N, T> value {};
    for (size_t i = 0; i < N; ++i)
    {
        value[ i ] = _p->_spinBoxArray[ i ]->max();
    }
    return value;
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setStep(glm::vec<N, T> step)
{
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setStep(step[ i ]);
    }
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::step() const
{
    glm::vec<N, T> value {};
    for (size_t i = 0; i < N; ++i)
    {
        value[ i ] = _p->_spinBoxArray[ i ]->step();
    }
    return value;
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setSingleStep(glm::vec<N, T> step)
{
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setSingleStep(step[ i ]);
    }
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::singleStep() const
{
    glm::vec<N, T> value {};
    for (size_t i = 0; i < N; ++i)
    {
        value[ i ] = _p->_spinBoxArray[ i ]->singleStep();
    }
    return value;
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setPageStep(glm::vec<N, T> step)
{
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setPageStep(step[ i ]);
    }
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::pageStep() const
{
    glm::vec<N, T> value {};
    for (size_t i = 0; i < N; ++i)
    {
        value[ i ] = _p->_spinBoxArray[ i ]->pageStep();
    }
    return value;
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setDecimals(int decimals)
{
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setDecimals(decimals);
    }
}

template <size_t N, typename T>
int MultiSpinBox<N, T>::decimals() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    return _p->_spinBoxArray[ 0 ]->decimals();
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setAccelerated(bool accelerated)
{
    for (size_t i = 0; i < N; ++i)
    {
        _p->_spinBoxArray[ i ]->setAccelerated(accelerated);
    }
}

template <size_t N, typename T>
bool MultiSpinBox<N, T>::isAccelerated() const
{
    // It's the same for all three spin boxes, so we can just return one of them
    // them.
    return _p->_spinBoxArray[ 0 ]->isAccelerated();
}

template <size_t N, typename T>
void MultiSpinBox<N, T>::setValue(glm::vec<N, T> value, bool force)
{
    if (force || (_p->_value != value))
    {
        _p->_value = value;
        for (size_t i = 0; i < N; ++i)
        {
            bool b = _p->_spinBoxArray[ i ]->blockSignals(true);
            _p->_spinBoxArray[ i ]->setValue(value[ i ]);
            _p->_spinBoxArray[ i ]->blockSignals(b);
        }
        value_changed(_p->_value);
    }
}

template <size_t N, typename T>
glm::vec<N, T> MultiSpinBox<N, T>::value()
{
    return _p->_value;
}

template class MultiSpinBox<2, int>;
template class MultiSpinBox<2, unsigned int>;
template class MultiSpinBox<2, float>;
template class MultiSpinBox<2, double>;
template class MultiSpinBox<3, int>;
template class MultiSpinBox<3, unsigned int>;
template class MultiSpinBox<3, float>;
template class MultiSpinBox<3, double>;
template class MultiSpinBox<4, int>;
template class MultiSpinBox<4, unsigned int>;
template class MultiSpinBox<4, float>;
template class MultiSpinBox<4, double>;
} // namespace ui
