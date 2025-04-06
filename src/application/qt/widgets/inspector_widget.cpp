#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <common/logging.hpp>
#include <project/component_interface/component.hpp>
#include <project/game_object.hpp>
#include <qcombobox.h>

#include "inspector_widget.hpp"

#include "collapsible_widget.hpp"
#include "vec3_widget.hpp"

namespace ui
{
logger log() { return get_logger("inspector_widget"); }

struct InspectorWidget::InspectorWidgetPrivate
{
    std::shared_ptr<game_object> _gameObject;
};

InspectorWidget::InspectorWidget(QWidget* parent)
    : QScrollArea(parent)
    , _p(std::make_unique<InspectorWidgetPrivate>())
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);

    QWidget* widget = new QWidget(this);
    auto layout = new QVBoxLayout();
    widget->setLayout(layout);
    setWidget(widget);
}

InspectorWidget::~InspectorWidget() = default;

void InspectorWidget::setInspectingObject(std::shared_ptr<game_object> t)
{
    if (_p->_gameObject != t)
    {
        _p->_gameObject = t;
        resetInspector();
    }
}

void InspectorWidget::resetInspector()
{
    if (_p->_gameObject == nullptr)
    {
        return;
    }

    auto layout = static_cast<QVBoxLayout*>(widget()->layout());

    _p->_gameObject->visit_components([ mainLayout = layout ](auto& component)
    {
        log()->debug("Component: {}", component.name());

        auto* collapsibleWidget = new CollapsibleWidget(QString::fromLatin1(
            component.name().data(), component.name().size()));
        mainLayout->addWidget(collapsibleWidget);
        auto layout = new QVBoxLayout();

        component.for_each_property(
            [ layout, &component ](std::string_view prop_name,
                                   std::string_view prop_display_name,
                                   const auto& prop_value) -> bool
        {
            std::string prop_name_str(prop_name);
            log()->debug("Property: {}({})", prop_display_name, prop_name_str);
            switch (prop_value.index())
            {
            case variant_index_v<trivial_types::variant_t, bool>:
            {
                QCheckBox* checkBox = new QCheckBox(QString::fromLatin1(
                    prop_display_name.data(), prop_display_name.size()));
                layout->addWidget(checkBox);
                checkBox->setChecked(std::get<bool>(prop_value) == true);
                checkBox->connect(checkBox,
                                  &QCheckBox::stateChanged,
                                  [ prop_name_str, &component ](auto state)
                {
                    component.set_property_value(prop_name_str,
                                                 state == Qt::Checked);
                });
                break;
            }
            case variant_index_v<trivial_types::variant_t, glm::dvec3>:
            {
                auto vec3 = std::get<glm::dvec3>(prop_value);
                auto vec3Widget = new ui::Vec3Widget();
                vec3Widget->setLabel(QString::fromLatin1(
                    prop_display_name.data(), prop_display_name.size()));
                vec3Widget->setValue(vec3);
                layout->addWidget(vec3Widget);
                vec3Widget->connect(vec3Widget,
                                    &ui::Vec3Widget::valueChanged,
                                    [ prop_name_str, &component ](auto value)
                {
                    component.set_property_value(prop_name_str,
                                                 glm::dvec3(value));
                });
                break;
            }
            default: break;
            }
            return true;
        });

        collapsibleWidget->setContentLayout(layout);
        layout->setSpacing(0);
        layout->addItem(new QSpacerItem(
            0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
        return true;
    });

    layout->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

QSize InspectorWidget::sizeHint() const { return QSize(200, 800); }
} // namespace ui
