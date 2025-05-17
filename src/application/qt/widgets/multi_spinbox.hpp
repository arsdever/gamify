#pragma once

#include <application/qt/widgets/spinbox.hpp>
#include <common/event.hpp>

namespace ui
{
template <size_t N, typename T>
class MultiSpinBox : public QWidget
{
public:
    MultiSpinBox(QWidget* parent = nullptr);
    ~MultiSpinBox() override;

    void setLabel(const QString& label);
    QString label() const;

    void setValue(glm::vec<N, T> value, bool force = false);
    glm::vec<N, T> value();

    void setRangeMin(glm::vec<N, T> min);
    void setRangeMax(glm::vec<N, T> max);
    glm::vec<N, T> min() const;
    glm::vec<N, T> max() const;
    void setStep(glm::vec<N, T> step);
    glm::vec<N, T> step() const;
    void setSingleStep(glm::vec<N, T> step);
    glm::vec<N, T> singleStep() const;
    void setPageStep(glm::vec<N, T> step);
    glm::vec<N, T> pageStep() const;
    void setDecimals(int decimals);
    int decimals() const;
    void setAccelerated(bool accelerated);
    bool isAccelerated() const;

    ::event<void(glm::vec<N, T>)> value_changed;

private:
    struct impl;
    std::unique_ptr<impl> _p;
};

class Vec3Widget : public MultiSpinBox<3, double>
{
    Q_OBJECT
public:
    inline explicit Vec3Widget(QWidget* parent = nullptr)
    {
        value_changed += [ this ](auto value) { emit valueChanged(value); };
    }

signals:
    void valueChanged(glm::dvec3 value);
};
} // namespace ui
