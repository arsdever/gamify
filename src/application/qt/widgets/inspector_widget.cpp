#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <assets/asset_manager.hpp>
#include <common/logging.hpp>
#include <graphics/graphics_fwd.hpp>
#include <project/component_interface/component.hpp>
#include <project/game_object.hpp>

#include "inspector_widget.hpp"

#include "collapsible_widget.hpp"
#include "project/property.hpp"
#include "spinbox.hpp"
#include "vec3_widget.hpp"

namespace ui
{
logger log() { return get_logger("inspector_widget"); }

struct InspectorWidget::InspectorWidgetPrivate
{
    std::shared_ptr<game_object> _gameObject;
    QWidget* _inspectorContent { nullptr };
};

InspectorWidget::InspectorWidget(QWidget* parent)
    : QScrollArea(parent)
    , _p(std::make_unique<InspectorWidgetPrivate>())
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
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

template <typename T>
struct TypedUiBuildHandler
{
    template <typename... Args>
    static bool try_handle(class property& prop, Args&&... args)
    {
        if (prop.get_type() != typeid(T).name())
        {
            return false;
        }

        handle(prop, std::forward<Args>(args)...);
        return true;
    }

    template <typename... Args>
    static void handle(Args&&... args);
};

template <>
template <>
void TypedUiBuildHandler<bool>::handle<class property&, QLayout*>(
    class property& prop, QLayout*&& layout)
{
    QCheckBox* checkBox =
        new QCheckBox(QString::fromStdString(prop.get_display_name()));
    layout->addWidget(checkBox);
    checkBox->setChecked(prop.get_value<bool>() == true);
    checkBox->connect(checkBox,
                      &QCheckBox::stateChanged,
                      [ &prop ](auto state)
    { prop.set_value(state == Qt::Checked); });
    auto connection = prop.value_changed += [ &prop, checkBox ]()
    {
        auto b = checkBox->blockSignals(true);
        checkBox->setChecked(prop.get_value<bool>());
        checkBox->blockSignals(b);
    };
    checkBox->connect(checkBox,
                      &QWidget::destroyed,
                      [ c = std::move(connection) ](auto) mutable
    { c.drop(); });
}

template <>
template <>
void TypedUiBuildHandler<glm::dvec3>::handle<class property&, QLayout*>(
    class property& prop, QLayout*&& layout)
{
    auto value = prop.get_value<glm::dvec3>();
    auto vec3Widget = new ui::Vec3Widget();
    vec3Widget->setLabel(QString::fromLatin1(prop.get_display_name()));
    vec3Widget->setValue(value);
    layout->addWidget(vec3Widget);
    vec3Widget->connect(vec3Widget,
                        &ui::Vec3Widget::valueChanged,
                        [ &prop ](auto value) { prop.set_value(value); });
    auto connection = prop.value_changed += [ &prop, vec3Widget ]()
    {
        auto b = vec3Widget->blockSignals(true);
        vec3Widget->setValue(prop.get_value<glm::dvec3>());
        vec3Widget->blockSignals(b);
    };
    vec3Widget->connect(vec3Widget,
                        &QWidget::destroyed,
                        [ c = std::move(connection) ](auto) mutable
    { c.drop(); });
}

template <>
template <>
void TypedUiBuildHandler<double>::handle<class property&, QLayout*>(
    class property& prop, QLayout*&& layout)
{
    auto value = prop.get_value<double>();
    auto spin = new ui::SpinBox();
    spin->setLabel(QString::fromLatin1(prop.get_display_name()));
    spin->setValue(value);
    layout->addWidget(spin);
    spin->connect(spin,
                  &ui::SpinBox::valueChanged,
                  [ &prop ](auto value) { prop.set_value(value); });
    auto connection = prop.value_changed += [ &prop, spin ]()
    {
        auto b = spin->blockSignals(true);
        spin->setValue(prop.get_value<double>());
        spin->blockSignals(b);
    };
    spin->connect(spin,
                  &QWidget::destroyed,
                  [ c = std::move(connection) ](auto) mutable { c.drop(); });
}

template <>
template <>
void TypedUiBuildHandler<std::shared_ptr<graphics::mesh>>::
    handle<class property&, QLayout*>(class property& prop, QLayout*&& layout)
{
    auto value = prop.get_value<std::shared_ptr<graphics::mesh>>();
    QComboBox* comboBox = new QComboBox();
    assets::asset_manager::apply<graphics::mesh>(
        [ comboBox, &value ](std::string_view name,
                             std::shared_ptr<assets::asset> ast)
    {
        comboBox->addItem(QString::fromLatin1(name), QVariant::fromValue(ast));
        if (ast->as<graphics::mesh>() == value)
        {
            comboBox->setCurrentText(QString::fromLatin1(name));
        }
    });
    layout->addWidget(comboBox);
    auto connection = prop.value_changed += [ &prop ]()
    {
        // TODO: Need to find an asset via the mesh pointer
    };
    comboBox->connect(comboBox,
                      &QComboBox::currentTextChanged,
                      [ &prop, comboBox ](auto text)
    {
        auto asset = comboBox->currentData(Qt::UserRole)
                         .value<std::shared_ptr<assets::asset>>();
        if (asset)
        {
            prop.set_value(asset->as<graphics::mesh>());
        }
    });
    comboBox->connect(comboBox,
                      &QWidget::destroyed,
                      [ c = std::move(connection) ](auto) mutable
    { c.drop(); });
}

using SupportedUiTypes =
    std::tuple<bool, glm::dvec3, double, std::shared_ptr<graphics::mesh>>;

void InspectorWidget::resetInspector()
{
    if (_p->_gameObject == nullptr)
    {
        return;
    }

    QWidget* widget = new QWidget(this);
    setWidget(widget);
    auto layout = new QVBoxLayout;
    widget->setLayout(layout);

    _p->_gameObject->visit_components([ mainLayout = layout ](auto& component)
    {
        log()->debug("Component: {}", component.name());

        auto* collapsibleWidget = new CollapsibleWidget(QString::fromLatin1(
            component.name().data(), component.name().size()));
        mainLayout->addWidget(collapsibleWidget);
        auto layout = new QVBoxLayout();

        component.for_each_property(
            [ layout, &component ](class property& prop) -> bool
        {
            log()->debug("Property: {}({}) of type {}",
                         prop.get_display_name(),
                         prop.get_name(),
                         prop.get_type());

            constexpr size_t supported_types_count =
                std::tuple_size_v<SupportedUiTypes>;

            bool handled = false;

            [ & ]<std::size_t... I>(std::index_sequence<I...>)
            {
                ((handled =
                      handled ||
                      TypedUiBuildHandler<
                          std::tuple_element_t<I, SupportedUiTypes>>::
                          try_handle(prop, qobject_cast<QLayout*>(layout))),
                 ...);
            }(std::make_index_sequence<supported_types_count> {});

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
