#include <common/logging.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "project/components/mesh_renderer.hpp"

#include "project/project_manager.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

inline logger log() { return get_logger("mesh_renderer"); }

namespace components
{
mesh_renderer::mesh_renderer(game_object& obj)
    : component(type_name, obj)
{
    add_property({ "materials",
                   "Materials",
                   "The list of materials for each submesh",
                   std::shared_ptr<graphics::material> {} });
}

std::vector<std::shared_ptr<graphics::material>>
mesh_renderer::get_materials() const
{
    return { get_property("materials")
                 .get_value<const std::shared_ptr<graphics::material>&>() };
}

void mesh_renderer::set_materials(
    std::vector<std::shared_ptr<graphics::material>> m)
{
    get_materials() = std::move(m);
}

std::shared_ptr<graphics::material>
mesh_renderer::get_material(size_t index) const
{
    return get_materials()[ std::min(index, get_materials().size() - 1) ];
}

void mesh_renderer::add_material(std::shared_ptr<graphics::material> m)
{
    if (m == nullptr)
    {
        log()->warn("{}: Adding null material to mesh renderer", get_name());
    }
    // get_materials().push_back(m);
    get_property("materials") = m;
}

void mesh_renderer::set_material(size_t index,
                                 std::shared_ptr<graphics::material> m)
{
    // if (index >= get_materials().size())
    // {
    //     get_materials().resize(index + 1);
    // }

    // get_materials()[ index ] = m;

    // if (m == nullptr)
    // {
    //     log()->warn("{}: Setting a null material to mesh renderer at index
    //     {}",
    //                 get_name(),
    //                 index);
    // }
    get_property("materials") = m;
}

template <>
void mesh_renderer::serialize<json_serializer>(json_serializer& s)
{
    s.add_component(nlohmann::json {
        { "type", mesh_renderer::type_name },
        { "is_enabled", is_enabled() },
    });
}

void mesh_renderer::deserialize(const nlohmann::json& j)
{
    set_enabled(j[ "is_enabled" ]);
}
} // namespace components
