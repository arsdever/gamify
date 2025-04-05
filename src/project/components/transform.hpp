#pragma once

#include <nlohmann/json_fwd.hpp>

#include "project/project_fwd.hpp"

#include "common/event.hpp"
#include "project/component_interface/component.hpp"

namespace components
{
class transform : public component
{
public:
    enum class relation_flag
    {
        local,
        world
    };

public:
    using base = component;

    transform(game_object& obj);

    void set_position(const glm::dvec3& position);
    void set_rotation(const glm::dvec3& euler);
    void set_rotation(const glm::dquat& rotation);
    void set_scale(const glm::dvec3& scale);

    template <relation_flag F = relation_flag::local>
    glm::dvec3 get_position() const;
    template <relation_flag F = relation_flag::local>
    glm::dquat get_rotation() const;
    template <relation_flag F = relation_flag::local>
    glm::dvec3 get_scale() const;
    template <relation_flag F = relation_flag::local>
    glm::dmat4 get_matrix() const;

    glm::dvec3 get_forward() const;
    glm::dvec3 get_right() const;
    glm::dvec3 get_up() const;

    bool is_updated() const;

    void move(glm::dvec3 offset);
    void rotate(const glm::dvec3& axis, double angle);

    void set_property_value(std::string_view name,
                            trivial_types::variant_t value) override;
    void for_each_property(const property_visitor_type& visitor) const override;

    // static size_t register_component();
    // TODO: this is a temporary solution for resolving the type_name
    static constexpr std::string_view type_name = "transform";

    template <typename ST>
    void serialize(ST& s);

    void deserialize(const nlohmann::json& j);

    event<void(const glm::dvec3&)> on_position_changed;
    event<void(const glm::dquat&)> on_rotation_changed;
    event<void(const glm::dvec3&)> on_scale_changed;

protected:
    void on_update() override;

private:
    bool is_dirty() const;
    bool recalculate_matrix() const;

private:
    glm::dvec3 _position { 0 };
    glm::dquat _rotation { glm::identity<glm::dquat>() };
    glm::dvec3 _scale { 1 };
    mutable glm::dvec3 _world_position { 0 };
    mutable glm::dquat _world_rotation { glm::identity<glm::dquat>() };
    mutable glm::dvec3 _world_scale { 1 };
    mutable glm::dmat4 _matrix { glm::identity<glm::dmat4>() };
    mutable glm::dmat4 _world_matrix { glm::identity<glm::dmat4>() };
    mutable bool _dirty { false };
    mutable bool _updated { true };
};
} // namespace components
