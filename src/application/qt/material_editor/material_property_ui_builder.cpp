#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
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

#include "application/qt/widgets/color_widget.hpp"
#include "application/qt/widgets/texture_widget.hpp"
#include "assets/asset.hpp"
#include "common/main_thread_dispatcher.hpp"
#include "tools/material_viewer/material_viewer.hpp"

struct material_property_ui_builder::material_property_ui_builder_impl
{
    static void create_control(QGridLayout* layout,
                               int& row,
                               std::string_view property_name,
                               const std::any& property_value,
                               std::function<void(std::any)> property_setter)
    {
        if (property_value.type() == typeid(std::tuple<float>))
        {
            auto slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(10000);
            slider->setSingleStep(1);
            slider->setPageStep(10);
            slider->setMinimumWidth(200);
            slider->setSizePolicy(QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Maximum);
            slider->connect(slider,
                            &QSlider::valueChanged,
                            [ property_setter ](int value)
            { property_setter(value / 10000.0f); });
            QDoubleSpinBox* spinbox = new QDoubleSpinBox();
            spinbox->setMinimum(0.0f);
            spinbox->setMaximum(1.0f);
            spinbox->setSingleStep(0.0001f);
            spinbox->setMinimumWidth(100);
            spinbox->setDecimals(4);
            spinbox->connect(spinbox,
                             &QDoubleSpinBox::valueChanged,
                             [ property_setter, slider ](float value)
            { slider->setValue(value * 10000.0f); });
            spinbox->connect(slider,
                             &QSlider::valueChanged,
                             spinbox,
                             [ spinbox ](int value)
            {
                auto bs = spinbox->blockSignals(true);
                spinbox->setValue(value / 10000.0f);
                spinbox->blockSignals(bs);
            });
            spinbox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            slider->setValue(
                std::get<0>(std::any_cast<std::tuple<float>>(property_value)) *
                10000.0f);
            layout->addWidget(spinbox, row, 1, 1, 1);
            layout->addWidget(slider, row, 2, 1, -1);
        }
        else if (property_value.type() ==
                 typeid(std::shared_ptr<graphics::texture>))
        {
            auto texture = std::any_cast<std::shared_ptr<graphics::texture>>(
                property_value);

            ui::texture_widget* wdg = new ui::texture_widget();
            wdg->connect(wdg,
                         &ui::texture_widget::image_changed,
                         [ wdg, property_setter ](std::string_view path)
            {
                auto txt = assets::asset_manager::get<graphics::texture>(
                    assets::asset_manager::get_asset_key_by_path(path));

                property_setter(txt);

                if (!txt)
                {
                    return;
                }

                wdg->set_image(txt);
            });
            wdg->set_image(texture);
            layout->addWidget(wdg, row, 1, 1, -1);
        }
        else if (property_value.type() ==
                 typeid(std::tuple<float, float, float, float>))
        {
            ui::color_widget* wdg = new ui::color_widget();
            auto [ r, g, b, a ] =
                std::any_cast<std::tuple<float, float, float, float>>(
                    property_value);
            glm::vec4 color = { r, g, b, a };
            wdg->set_color(color);
            wdg->connect(wdg,
                         &ui::color_widget::color_changed,
                         [ property_setter ](glm::vec4 color)
            {
                property_setter(
                    std::make_tuple(color.r, color.g, color.b, color.a));
            });
            layout->addWidget(wdg, row, 1, 1, -1);
        }

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

    layout->addWidget(mesh_selector_label, row, 0, 1, 1);
    layout->addWidget(mesh_selector, row++, 1, 1, -1);
    layout->addWidget(material_selector_label, row, 0, 1, 1);
    layout->addWidget(material_selector, row++, 1, 1, -1);

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
