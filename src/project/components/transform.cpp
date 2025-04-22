#include <glm/gtx/matrix_decompose.hpp>

#include "project/components/transform.hpp"

#include "project/project_manager.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

namespace components
{
transform::transform(game_object& obj)
    : component(type_name, obj)
{
    _properties.emplace("position",
                        property { "position",
                                   "Position",
                                   "Position of the object in world space",
                                   glm::dvec3 { 0, 0, 0 } });
    _properties.emplace("rotation",
                        property { "rotation",
                                   "Rotation",
                                   "Rotation of the object in world space (euler angles)",
                                   glm::dvec3 { 0, 0, 0 } });
    _properties.emplace("scale",
                        property { "scale",
                                   "Scale",
                                   "Scale of the object in world space",
                                   glm::dvec3 { 1, 1, 1 } });
}

void transform::set_position(const glm::dvec3& position)
{
    _position = position;
    _dirty = true;
    _updated = true;
    on_position_changed(_position);
}

void transform::set_rotation(const glm::dquat& rotation)
{
    _rotation = rotation;
    _dirty = true;
    _updated = true;
    on_rotation_changed(_rotation);
}

void transform::set_rotation(const glm::dvec3& rotation)
{
    _rotation = glm::dquat(rotation);
    _dirty = true;
    _updated = true;
    on_rotation_changed(_rotation);
}

void transform::set_scale(const glm::dvec3& scale)
{
    _scale = scale;
    _dirty = true;
    _updated = true;
    on_scale_changed(_scale);
}

template <>
glm::dmat4 transform::get_matrix<transform::relation_flag::local>() const
{
    recalculate_matrix();
    return _matrix;
}

template <>
glm::dmat4 transform::get_matrix<transform::relation_flag::world>() const
{
    recalculate_matrix();
    return _world_matrix;
}

template <>
glm::dvec3 transform::get_position<transform::relation_flag::local>() const
{
    return _position;
}

template <>
glm::dquat transform::get_rotation<transform::relation_flag::local>() const
{
    return _rotation;
}

template <>
glm::dvec3 transform::get_scale<transform::relation_flag::local>() const
{
    return _scale;
}

template <>
glm::dvec3 transform::get_position<transform::relation_flag::world>() const
{
    recalculate_matrix();
    return _world_position;
}

template <>
glm::dquat transform::get_rotation<transform::relation_flag::world>() const
{
    recalculate_matrix();
    return _world_rotation;
}

template <>
glm::dvec3 transform::get_scale<transform::relation_flag::world>() const
{
    recalculate_matrix();
    return _world_scale;
}

glm::dvec3 transform::get_forward() const
{
    return glm::normalize(glm::dvec3(get_matrix<relation_flag::local>() *
                                     glm::dvec4(0, 0, 1, 0)));
}

glm::dvec3 transform::get_right() const
{
    return glm::normalize(glm::dvec3(get_matrix<relation_flag::local>() *
                                     glm::dvec4(1, 0, 0, 0)));
}

glm::dvec3 transform::get_up() const
{
    return glm::normalize(glm::dvec3(get_matrix<relation_flag::local>() *
                                     glm::dvec4(0, 1, 0, 0)));
}

bool transform::is_updated() const { return _updated; }

void transform::move(glm::dvec3 offset) { set_position(_position + offset); }

void transform::rotate(const glm::dvec3& axis, double angle)
{
    set_rotation(glm::angleAxis(angle, axis) * _rotation);
}

template <>
void transform::serialize<json_serializer>(json_serializer& s)
{
    s.add_component(nlohmann::json { { "type", transform::type_name },
                                     { "is_enabled", is_enabled() },
                                     { "transform",
                                       {
                                           { "position", to_json(_position) },
                                           { "rotation", to_json(_rotation) },
                                           { "scale", to_json(_position) },
                                       } } });
}

void transform::deserialize(const nlohmann::json& j)
{
    _position = { j[ "position" ][ 0 ],
                  j[ "position" ][ 1 ],
                  j[ "position" ][ 2 ] };

    _rotation = { j[ "rotation" ][ 3 ],
                  j[ "rotation" ][ 0 ],
                  j[ "rotation" ][ 1 ],
                  j[ "rotation" ][ 2 ] };

    _scale = { j[ "scale" ][ 0 ], j[ "scale" ][ 1 ], j[ "scale" ][ 2 ] };

    set_enabled(j[ "is_enabled" ]);
}

bool transform::is_dirty() const
{
    if (!get_game_object().has_parent())
        return _dirty;

    return get_game_object().get_parent()->get_transform().is_dirty();
}

bool transform::recalculate_matrix() const
{
    bool world_matrix_changed = false;
    if (_dirty)
    {
        _matrix = glm::identity<glm::dmat4>();
        _matrix = glm::translate(_matrix, _position) *
                  glm::mat4_cast(_rotation) * glm::scale(_matrix, _scale);
        world_matrix_changed = true;
        _dirty = false;
    }

    if (get_game_object().has_parent())
    {
        if (get_game_object()
                .get_parent()
                ->get_transform()
                .recalculate_matrix() &&
            !world_matrix_changed)
        {
            return false;
        }
        _world_matrix =
            get_game_object().get_parent()->get_transform().get_matrix() *
            _matrix;
    }
    else
    {
        if (world_matrix_changed)
            _world_matrix = _matrix;
    }

    if (world_matrix_changed)
    {
        glm::dvec3 skew;
        glm::dvec4 perspective;
        glm::decompose(_world_matrix,
                       _world_scale,
                       _world_rotation,
                       _world_position,
                       skew,
                       perspective);
    }

    return world_matrix_changed;
}

void transform::on_update() { _updated = false; }
} // namespace components
