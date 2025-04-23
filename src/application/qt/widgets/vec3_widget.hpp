#pragma once

#include <QWidget>

namespace ui
{
class SpinBox;

class Vec3Widget : public QWidget
{
    Q_OBJECT
public:
    Vec3Widget(QWidget* parent = nullptr);
    ~Vec3Widget();

    void setLabel(const QString& label);
    QString label() const;

    void setValue(glm::dvec3 value, bool force = false);
    glm::dvec3 value();

    void setRangeMin(glm::dvec3 min);
    void setRangeMax(glm::dvec3 max);
    glm::dvec3 min() const;
    glm::dvec3 max() const;
    void setStep(glm::dvec3 step);
    glm::dvec3 step() const;
    void setSingleStep(glm::dvec3 step);
    glm::dvec3 singleStep() const;
    void setPageStep(glm::dvec3 step);
    glm::dvec3 pageStep() const;
    void setDecimals(int decimals);
    int decimals() const;
    void setAccelerated(bool accelerated);
    bool isAccelerated() const;

signals:
    void valueChanged(glm::dvec3 color);

private:
    struct Vec3WidgetPrivate;
    std::unique_ptr<Vec3WidgetPrivate> _p;
};
} // namespace ui
