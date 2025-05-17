#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "project/components/transform.hpp"

#include "project/project_manager.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

namespace components
{
template <>
glm::dvec3 transform::get_position<transform::relation_flag::local>() const
{
    return param_position().get_value<glm::dvec3>();
}

template <>
glm::dquat transform::get_rotation<transform::relation_flag::local>() const
{
    return glm::quat(param_rotation().get_value<glm::dvec3>());
}

template <>
glm::dvec3 transform::get_scale<transform::relation_flag::local>() const
{
    return param_scale().get_value<glm::dvec3>();
}

transform::transform(game_object& obj)
    : component(type_name, obj)
{
    auto& pos = add_property({ "position",
                               "Position",
                               "Position of the object in world space",
                               glm::dvec3 { 0, 0, 0 } });
    auto& rot =
        add_property({ "rotation",
                       "Rotation",
                       "Rotation of the object in world space (euler angles)",
                       glm::dvec3 { 0, 0, 0 } });
    auto& scale = add_property({ "scale",
                                 "Scale",
                                 "Scale of the object in world space",
                                 glm::dvec3 { 1, 1, 1 } });

    pos.value_changed += [ this ]()
    {
        _dirty = true;
        _updated = true;
        on_position_changed(get_position<relation_flag::local>());
    };

    rot.value_changed += [ this ]()
    {
        _dirty = true;
        _updated = true;
        on_rotation_changed(get_rotation<relation_flag::local>());
    };

    scale.value_changed += [ this ]()
    {
        _dirty = true;
        _updated = true;
        on_scale_changed(get_scale<relation_flag::local>());
    };
}

void transform::set_position(const glm::dvec3& position)
{
    param_position() = position;
    _dirty = true;
    _updated = true;
    on_position_changed(get_position<relation_flag::local>());
}

void transform::set_rotation(const glm::dquat& rotation)
{
    param_rotation() = glm::eulerAngles(rotation);
    _dirty = true;
    _updated = true;
    on_rotation_changed(get_rotation<relation_flag::local>());
}

void transform::set_rotation(const glm::dvec3& rotation)
{
    param_rotation() = rotation;
    _dirty = true;
    _updated = true;
    on_rotation_changed(get_rotation<relation_flag::local>());
}

void transform::set_scale(const glm::dvec3& scale)
{
    param_scale() = scale;
    _dirty = true;
    _updated = true;
    on_scale_changed(get_scale<relation_flag::local>());
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

void transform::move(glm::dvec3 offset)
{
    set_position(get_position<relation_flag::local>() + offset);
}

void transform::rotate(const glm::dvec3& axis, double angle)
{
    set_rotation(glm::angleAxis(angle, axis) *
                 get_rotation<relation_flag::local>());
}

// template <>
// void transform::serialize<json_serializer>(json_serializer& s)
// {
//     s.add_component(nlohmann::json { { "type", transform::type_name },
//                                      { "is_enabled", is_enabled() },
//                                      { "transform",
//                                        {
//                                            { "position",
//                                            to_json(_position)
//                                            }, { "rotation",
//                                            to_json(_rotation) }, {
//                                            "scale", to_json(_position) },
//                                        } } });
// }

// void transform::deserialize(const nlohmann::json& j)
// {
//     _position = { j[ "position" ][ 0 ],
//                   j[ "position" ][ 1 ],
//                   j[ "position" ][ 2 ] };

//     _rotation = { j[ "rotation" ][ 3 ],
//                   j[ "rotation" ][ 0 ],
//                   j[ "rotation" ][ 1 ],
//                   j[ "rotation" ][ 2 ] };

//     _scale = { j[ "scale" ][ 0 ], j[ "scale" ][ 1 ], j[ "scale" ][ 2 ] };

//     set_enabled(j[ "is_enabled" ]);
// }

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
        _matrix =
            glm::translate(_matrix, get_position<relation_flag::local>()) *
            glm::mat4_cast(get_rotation<relation_flag::local>()) *
            glm::scale(_matrix, get_scale<relation_flag::local>());
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

property& transform::param_position() { return get_property("position"); }

property& transform::param_rotation() { return get_property("rotation"); }

property& transform::param_scale() { return get_property("scale"); }

const property& transform::param_position() const
{
    return get_property("position");
}

const property& transform::param_rotation() const
{
    return get_property("rotation");
}

const property& transform::param_scale() const { return get_property("scale"); }
} // namespace components
