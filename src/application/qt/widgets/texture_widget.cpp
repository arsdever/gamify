#include <QColorDialog>
#include <QPushButton>

#include <graphics/texture.hpp>
#include <qfiledialog.h>

#include "application/qt/widgets/texture_widget.hpp"

static constexpr auto THUMBNAIL_SIZE = 80;

namespace ui
{
texture_widget::texture_widget(QWidget* parent)
    : QPushButton(parent)
{
    clear_icon();
    setIconSize(QSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(
        this, &QPushButton::clicked, this, &texture_widget::request_new_image);
}

void texture_widget::set_image(std::shared_ptr<graphics::texture> txt)
{
    _current_image = txt;

    if (_current_image == nullptr)
    {
        clear_icon();
        return;
    }

    auto format = QImage::Format_RGBA8888;
    switch (txt->get_channel_count())
    {
    case 1: format = QImage::Format_Grayscale8; break;
    case 3: format = QImage::Format_RGB888; break;
    case 4: format = QImage::Format_RGBA8888; break;
    default:
    }
    QPixmap icon =
        QPixmap::fromImage(QImage(txt->raw_data<uchar>(),
                                  txt->get_width(),
                                  txt->get_height(),
                                  format))
            .scaled(THUMBNAIL_SIZE, THUMBNAIL_SIZE, Qt::KeepAspectRatio);
    setIcon(icon);
}

void texture_widget::request_new_image()
{
    auto path = QFileDialog::getOpenFileName(
        this, "Select texture", "", "Image files (*.png *.jpg *.jpeg)");

    if (path.isEmpty())
    {
        return;
    }

    emit image_changed(path.toStdString());
}

void texture_widget::clear_icon()
{
    static const QImage transparent(":/transparent.png");
    auto pixmap = QPixmap::fromImage(transparent);
    setIcon(pixmap);
}
} // namespace ui
