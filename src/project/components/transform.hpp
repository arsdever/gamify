#pragma once

#include <nlohmann/json_fwd.hpp>
#include "project/components/component.hpp"

class game_object;

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
    transform(game_object& obj);

    static transform& create(game_object& obj);
    static transform& get(const game_object& obj);

    void set_position(const glm::dvec3& position);
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

    static size_t register_component();

    template <typename ST>
    void serialize(ST& s);

    void deserialize(const nlohmann::json& j);

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
};
} // namespace components