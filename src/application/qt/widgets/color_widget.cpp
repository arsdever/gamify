#include <QColorDialog>
#include <QPushButton>

#include "application/qt/widgets/color_widget.hpp"

namespace ui
{
color_widget::color_widget(QWidget* parent)
    : QPushButton(parent)
{
    connect(
        this, &QPushButton::clicked, this, &color_widget::request_new_color);

    setAutoFillBackground(true);
    set_color(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), true);
}

void color_widget::set_color(glm::vec4 color, bool force)
{
    if (force | (_current_color != color))
    {
        _current_color = color;
        auto size = std::min(width(), height());
        QPalette pal = palette();
        pal.setColor(QPalette::Button,
                     QColor(_current_color.r * 255,
                            _current_color.g * 255,
                            _current_color.b * 255,
                            _current_color.a * 255));
        setPalette(pal);
        update();
        emit color_changed(_current_color);
    }
}

void color_widget::request_new_color()
{
    auto color = QColorDialog::getColor(QColor(_current_color.r * 255,
                                               _current_color.g * 255,
                                               _current_color.b * 255,
                                               _current_color.a * 255),
                                        this);
    if (color.isValid())
    {
        set_color(glm::vec4(
            color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    }
}
} // namespace ui
