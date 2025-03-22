#pragma once

#include <QPushButton>

namespace ui
{
class color_widget : public QPushButton
{
    Q_OBJECT
public:
    color_widget(QWidget* parent = nullptr);
    ~color_widget() = default;

    void set_color(glm::vec4 color, bool force = false);

signals:
    void color_changed(glm::vec4 color);

private slots:
    void request_new_color();

private:
    glm::vec4 _current_color;
};
} // namespace ui
