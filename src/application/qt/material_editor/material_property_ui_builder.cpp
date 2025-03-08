#include <QColorDialog>
#include <QComboBox>
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
#include "tools/material_viewer/material_viewer.hpp"

struct material_property_ui_builder::material_property_ui_builder_impl
{
    static QWidget* get_control_for(std::function<void(std::any)> value_setter,
                                    const std::any& property_value)
    {
        if (property_value.type() == typeid(std::tuple<float>))
        {
            auto slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(10000);
            slider->setSingleStep(1);
            slider->setPageStep(10);
            slider->setMinimumWidth(200);
            slider->connect(slider,
                            &QSlider::valueChanged,
                            [ value_setter ](int value)
            { value_setter(value / 10000.0f); });
            slider->setValue(
                std::get<0>(std::any_cast<std::tuple<float>>(property_value)) *
                10000.0f);
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
                    [ value_setter, img_path, wdg ]
                {
                    std::any value;
                    auto txt = assets::asset_manager::get<graphics::texture>(
                        assets::asset_manager::get_asset_key_by_path(img_path));

                    if (txt)
                    {
                        value = txt;
                    }

                    value_setter(value);
                    if (!txt)
                    {
                        return;
                    }

                    QPushButton* wdg = new QPushButton();
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
                            .scaled(32, 32, Qt::KeepAspectRatio);
                    wdg->setIcon(icon);
                });
            });
            return wdg;
        }
        else if (property_value.type() ==
                 typeid(std::tuple<float, float, float, float>))
        {
            QPushButton* label = new QPushButton();
            QPixmap pixmap(32, 32);
            auto [ r, g, b, a ] =
                std::any_cast<std::tuple<float, float, float, float>>(
                    property_value);
            QColor c(r * 255, g * 255, b * 255, a * 255);
            pixmap.fill(c);
            label->setIcon(pixmap);
            label->connect(label,
                           &QPushButton::clicked,
                           [ value_setter, c, label ]()
            {
                auto color = QColorDialog::getColor(c, nullptr, "Select color");
                value_setter(std::tuple<float, float, float, float> {
                    color.redF(),
                    color.greenF(),
                    color.blueF(),
                    color.alphaF(),
                });
                QPixmap pixmap(32, 32);
                pixmap.fill(color);
                label->setIcon(pixmap);
            });
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
material_property_ui_builder::build(std::shared_ptr<material_viewer> viewer)
{
    auto mat = viewer->get_material();
    auto wmat = std::weak_ptr<graphics::material>(mat);
    auto wviewer = std::weak_ptr<material_viewer>(viewer);

    QWidget* main_widget = new QWidget();
    QGridLayout* layout = new QGridLayout(main_widget);
    main_widget->setLayout(layout);
    std::shared_ptr<graphics::material> active_material;

    int row = 0;

    mat->visit_properties(
        [ &row, layout, wmat, wviewer ](std::string_view property_name,
                                        const std::any& property_value)
    {
        material_property_ui_builder_impl::create_control(
            layout,
            row,
            property_name,
            property_value,
            [ wmat, wviewer, property_name ](std::any value)
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
        ++row;
    });

    QComboBox* mesh_selector = new QComboBox;
    assets::asset_manager::apply<graphics::mesh>(
        [ mesh_selector ](std::string_view name,
                          std::shared_ptr<assets::asset> ast)
    { mesh_selector->addItem(QString::fromLatin1(name)); });
    mesh_selector->connect(mesh_selector,
                           QOverload<int>::of(&QComboBox::currentIndexChanged),
                           [ mesh_selector, wviewer ](int index)
    {
        if (auto viewer = wviewer.lock())
        {
            auto key = assets::asset_manager::get_asset_key_by_path(
                mesh_selector->itemText(index).toStdString());
            auto msh = assets::asset_manager::get<graphics::mesh>(key);
            viewer->set_mesh(msh);
        }
    });
    QLabel* mesh_selector_label = new QLabel("Mesh");
    mesh_selector_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QComboBox* material_selector = new QComboBox;
    assets::asset_manager::apply<graphics::material>(
        [ material_selector ](std::string_view name,
                              std::shared_ptr<assets::asset> ast)
    { material_selector->addItem(QString::fromLatin1(name)); });
    material_selector->connect(
        material_selector,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [ material_selector, wviewer, &active_material ](int index)
    {
        auto key = assets::asset_manager::get_asset_key_by_path(
            material_selector->itemText(index).toStdString());
        auto mat = assets::asset_manager::get<graphics::material>(key);

        if (auto viewer = wviewer.lock())
        {
            viewer->set_material(mat);
        }

        active_material = mat;
    });
    QLabel* material_selector_label = new QLabel("Material");
    material_selector_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(mesh_selector_label, row, 0);
    layout->addWidget(mesh_selector, row++, 1);
    layout->addWidget(material_selector_label, row, 0);
    layout->addWidget(material_selector, row++, 1);

    QPushButton* save_button = new QPushButton("Save");
    // TODO: Implement the save action
    layout->addWidget(save_button, row++, 0, 1, -1);

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
