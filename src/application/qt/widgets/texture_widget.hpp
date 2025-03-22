#pragma once

#include <QPushButton>

#include <graphics/graphics_fwd.hpp>

namespace ui
{
class texture_widget : public QPushButton
{
    Q_OBJECT
public:
    texture_widget(QWidget* parent = nullptr);
    ~texture_widget() = default;

    void set_image(std::shared_ptr<graphics::texture> txt);

signals:
    void image_changed(std::string_view path);

private slots:
    void request_new_image();

private:
    void clear_icon();

private:
    std::shared_ptr<graphics::texture> _current_image;
};
} // namespace ui
