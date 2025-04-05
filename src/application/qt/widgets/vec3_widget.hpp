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

    void setValue(glm::vec3 value, bool force = false);
    glm::vec3 value();

    void setRangeMin(glm::vec3 min);
    void setRangeMax(glm::vec3 max);
    glm::vec3 min() const;
    glm::vec3 max() const;
    void setStep(glm::vec3 step);
    glm::vec3 step() const;
    void setSingleStep(glm::vec3 step);
    glm::vec3 singleStep() const;
    void setPageStep(glm::vec3 step);
    glm::vec3 pageStep() const;
    void setDecimals(int decimals);
    int decimals() const;
    void setAccelerated(bool accelerated);
    bool isAccelerated() const;

signals:
    void valueChanged(glm::vec3 color);

private:
    struct Vec3WidgetPrivate;
    std::unique_ptr<Vec3WidgetPrivate> _p;
};
} // namespace ui
