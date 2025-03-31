#pragma once

#include <QWidget>

class QDoubleSpinBox;

namespace ui
{
class Vec3Widget : public QWidget
{
    Q_OBJECT
public:
    Vec3Widget(QWidget* parent = nullptr);
    ~Vec3Widget() = default;

    void setValue(glm::vec3 value, bool force = false);
    glm::vec3 value();

signals:
    void valueChanged(glm::vec3 color);

private:
    glm::vec3 _value { 0 };
    QDoubleSpinBox* _xSpinBox { nullptr };
    QDoubleSpinBox* _ySpinBox { nullptr };
    QDoubleSpinBox* _zSpinBox { nullptr };
};
} // namespace ui
