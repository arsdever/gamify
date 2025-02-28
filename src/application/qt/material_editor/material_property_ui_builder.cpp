#include <QBoxLayout>
#include <QColorDialog>
#include <QLabel>
#include <QSlider>
#include <QWidget>

#include <graphics/material.hpp>

#include "material_property_ui_builder.hpp"

QWidget*
material_property_ui_builder::build(std::shared_ptr<graphics::material> mat)
{
    auto wmat = std::weak_ptr<graphics::material>(mat);

    QWidget* main_widget = new QWidget();
    main_widget->setLayout(new QVBoxLayout());

    mat->visit_properties([ main_widget, wmat ](std::string_view property_name,
                                                const std::any& property_value)
    {
        if (property_value.type() == typeid(float))
        {
            auto layout = new QHBoxLayout();
            auto label = new QLabel();
            layout->addWidget(label);
            label->setText(QString::fromLatin1(property_name));
            auto slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(100);
            slider->setSingleStep(1);
            slider->setValue(std::any_cast<float>(property_value) * 100);
            layout->addWidget(slider);
            main_widget->layout()->addItem(layout);
            slider->connect(slider,
                            &QSlider::valueChanged,
                            [ wmat, property_name ](int value)
            {
                if (auto mat = wmat.lock())
                    mat->set_property_value(property_name, value / 100.0f);
            });
        }
        else if (property_value.type() == typeid(std::tuple<float>))
        {
            auto layout = new QHBoxLayout();
            auto label = new QLabel();
            // layout->addWidget(label);
            label->setText(QString::fromLatin1(property_name));
            auto slider = new QSlider(Qt::Horizontal);
            slider->setMinimum(0);
            slider->setMaximum(100);
            slider->setSingleStep(1);
            slider->setValue(
                std::get<0>(std::any_cast<std::tuple<float>>(property_value)) *
                100);
            // layout->addWidget(slider);
            // main_widget->layout()->addItem(layout);
            main_widget->layout()->addWidget(label);
            main_widget->layout()->addWidget(slider);
            slider->connect(slider,
                            &QSlider::valueChanged,
                            [ wmat, property_name ](int value)
            {
                if (auto mat = wmat.lock())
                    mat->set_property_value(property_name, value / 100.0f);
            });
        }
    });

    return main_widget;
}
