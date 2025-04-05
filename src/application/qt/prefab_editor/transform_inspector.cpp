#include <QLabel>
#include <QVBoxLayout>

#include <application/qt/widgets/vec3_widget.hpp>
#include <project/components/transform.hpp>

#include "transform_inspector.hpp"

TransformInspector::TransformInspector(std::shared_ptr<components::transform> t,
                                       QWidget* parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);

    QWidget* widget = new QWidget(this);

    auto layout = new QVBoxLayout();
    widget->setLayout(layout);

    auto posWidget = new ui::Vec3Widget();
    auto rotWidget = new ui::Vec3Widget();
    auto scWidget = new ui::Vec3Widget();

    layout->addWidget(new QLabel("Position"));
    layout->addWidget(posWidget);
    posWidget->setDecimals(3);

    layout->addWidget(new QLabel("Rotation"));
    layout->addWidget(rotWidget);

    rotWidget->setSingleStep(glm::vec3 { M_PI / 360.0f });
    rotWidget->setStep(glm::vec3 { M_PI / 3.0f / 360.0f });
    rotWidget->setPageStep(glm::vec3 { M_PI / 30.0f });
    rotWidget->setRangeMin(glm::vec3 { -M_PI });
    rotWidget->setRangeMax(glm::vec3 { M_PI });
    rotWidget->setDecimals(3);

    layout->addWidget(new QLabel("Scale"));
    layout->addWidget(scWidget);
    scWidget->setDecimals(3);

    auto valueSetter = [](ui::Vec3Widget* widget, glm::vec3 value)
    {
        auto b = widget->blockSignals(true);
        widget->setValue(value, true);
        widget->blockSignals(b);
    };

    posWidget->connect(posWidget,
                       &ui::Vec3Widget::valueChanged,
                       [ t ](auto value) { t->set_position(value); });
    rotWidget->connect(rotWidget,
                       &ui::Vec3Widget::valueChanged,
                       [ t ](auto value) { t->set_rotation(value); });
    scWidget->connect(scWidget,
                      &ui::Vec3Widget::valueChanged,
                      [ t ](auto value) { t->set_scale(value); });

    valueSetter(posWidget,
                t->get_position<components::transform::relation_flag::local>());
    valueSetter(
        rotWidget,
        glm::eulerAngles(
            t->get_rotation<components::transform::relation_flag::local>()));
    valueSetter(scWidget,
                t->get_scale<components::transform::relation_flag::local>());

    _transform = t;
    _transform->on_position_changed += [ valueSetter, posWidget ](auto value)
    { valueSetter(posWidget, value); };
    _transform->on_rotation_changed += [ valueSetter, rotWidget ](auto value)
    { valueSetter(rotWidget, glm::eulerAngles(value)); };
    _transform->on_scale_changed +=
        [ valueSetter, scWidget ](auto value) { valueSetter(scWidget, value); };

    layout->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setWidget(widget);
}
