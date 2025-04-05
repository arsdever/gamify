#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include <common/logging.hpp>
#include <project/component_interface/component.hpp>
#include <project/game_object.hpp>

#include "inspector_widget.hpp"

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

    _p->_gameObject->visit_components(
        [ originalLayout = layout ](auto& component) -> bool
    {
        log()->debug("Component: {}", component.name());
        QFrame* frame = new QFrame();
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        frame->setLineWidth(1);
        frame->setMidLineWidth(1);

        originalLayout->addWidget(frame);

        auto layout = new QVBoxLayout(frame);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignTop);
        frame->setLayout(layout);

        layout->addWidget(new QLabel(QString::fromLatin1(
            component.name().data(), component.name().size())));
        component.for_each_property(
            [ layout, &component ](std::string_view prop_name,
                                   std::string_view prop_display_name,
                                   const auto& prop_value) -> bool
        {
            log()->debug("Property: {}({})", prop_display_name, prop_name);
            switch (prop_value.index())
            {
            case variant_index_v<trivial_types::variant_t, bool>:
            {
                QCheckBox* checkBox = new QCheckBox(QString::fromLatin1(
                    prop_display_name.data(), prop_display_name.size()));
                layout->addWidget(checkBox);
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
                                    [ prop_name, &component ](auto value)
                {
                    component.set_property_value(prop_name, glm::dvec3(value));
                });
                break;
            }
            default: break;
            }
            return true;
        });

        layout->addItem(new QSpacerItem(
            0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
        return true;
    });

    layout->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
} // namespace ui
