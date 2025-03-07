#include <QColorDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

#include <assets/asset_manager.hpp>
#include <graphics/graphics_fwd.hpp>
#include <graphics/material.hpp>
#include <graphics/texture.hpp>

#include "material_property_ui_builder.hpp"

#include "assets/asset.hpp"
#include "common/main_thread_dispatcher.hpp"

struct material_property_ui_builder::material_property_ui_builder_impl
{
    static QWidget* get_control_for(std::function<void(std::any)> value_setter,
                                    const std::any& property_value)
    {
        if (property_value.type() == typeid(std::tuple<float>))
        {
            auto slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(100);
            slider->setMinimumWidth(200);
            slider->connect(slider,
                            &QSlider::valueChanged,
                            [ value_setter ](int value)
            { value_setter(value / 100.0f); });
            return slider;
        }
        else if (property_value.type() ==
                 typeid(std::shared_ptr<graphics::texture>))
        {
            auto texture = std::any_cast<std::shared_ptr<graphics::texture>>(
                property_value);

            QImage img(":/transparent.png");

            if (texture)
            {
                img = QImage(texture->raw_data<uchar>(),
                             texture->get_width(),
                             texture->get_height(),
                             QImage::Format_RGBA8888);
            }
            QPushButton* wdg = new QPushButton();
            QPixmap icon = QPixmap::fromImage(img);
            icon = icon.scaled(32, 32, Qt::KeepAspectRatio);
            wdg->setIcon(icon);
            wdg->connect(wdg,
                         &QPushButton::clicked,
                         [ wdg, value_setter, texture ]()
            {
                auto img_path = QFileDialog::getOpenFileName(
                                    wdg,
                                    "Open image file",
                                    "",
                                    "Image files (*.png *.jpg *.jpeg)")
                                    .toStdString();

                common::main_thread_dispatcher::dispatch(
                    [ value_setter, img_path ]
                {
                    std::any value;
                    auto txt = assets::asset_manager::get<graphics::texture>(
                        assets::asset_manager::get_asset_key_by_path(img_path));

                    if (txt)
                    {
                        value = txt;
                    }

                    value_setter(value);
                });
            });
            return wdg;
        }
        else if (property_value.type() ==
                 typeid(std::tuple<float, float, float, float>))
        {
            QLabel* label = new QLabel();
            QPixmap pixmap(32, 32);
            auto [ r, g, b, a ] =
                std::any_cast<std::tuple<float, float, float, float>>(
                    property_value);
            QColor c(r * 255, g * 255, b * 255, a * 255);
            pixmap.fill(c);
            label->setPixmap(pixmap);
            label->setStyleSheet("border: 1px solid black;");
            return label;
        }
        return nullptr;
    }

    static void create_control(QGridLayout* layout,
                               int row,
                               std::string_view property_name,
                               const std::any& property_value,
                               std::function<void(std::any)> property_setter)
    {
        QWidget* control = get_control_for([ property_setter ](std::any value)
        { property_setter(value); },
                                           property_value);

        if (!control)
            return;

        layout->addWidget(control, row, 1);

        auto label = new QLabel();
        label->setText(QString::fromLatin1(property_name));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        layout->addWidget(label, row, 0);
    }
};

QWidget*
material_property_ui_builder::build(std::shared_ptr<graphics::material> mat)
{
    auto wmat = std::weak_ptr<graphics::material>(mat);

    QWidget* main_widget = new QWidget();
    QGridLayout* layout = new QGridLayout(main_widget);
    main_widget->setLayout(layout);

    int row = 0;

    mat->visit_properties([ &row, layout, wmat ](std::string_view property_name,
                                                 const std::any& property_value)
    {
        material_property_ui_builder_impl::create_control(
            layout,
            row,
            property_name,
            property_value,
            [ wmat, property_name ](std::any value)
        {
            auto action = [ wmat, &property_name, value = std::move(value) ]
            {
                if (auto mat = wmat.lock())
                    mat->set_property_value(property_name, std::move(value));
            };

            if (common::main_thread_dispatcher::is_main_thread())
            {
                action();
            }
            else
            {
                common::main_thread_dispatcher::dispatch([ action ]
                { action(); });
            }
        });
        // slider->connect(slider,
        //                 &QSlider::valueChanged,
        //                 [ wmat, property_name ](int value)
        // {
        //     if (auto mat = wmat.lock())
        //         mat->set_property_value(property_name, value / 100.0f);
        // });
        ++row;
    });
    layout->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
        row,
        0,
        1,
        2);

    return main_widget;
}

std::unique_ptr<material_property_ui_builder::material_property_ui_builder_impl>
    material_property_ui_builder::_impl { nullptr };
