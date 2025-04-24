#include <glm/gtx/matrix_decompose.hpp>

#include "project/components/mesh_filter.hpp"

#include "project/project_manager.hpp"
#include "project/serialization_utilities.hpp"
#include "project/serializer.hpp"
#include "project/serializer_json.hpp"

using namespace serialization::utilities;

namespace components
{
mesh_filter::mesh_filter(game_object& obj)
    : component(type_name, obj)
{
    add_property({ "mesh",
                   "Mesh",
                   "Mesh to render",
                   std::shared_ptr<graphics::mesh> { nullptr } });
}

std::shared_ptr<graphics::mesh> mesh_filter::get_mesh() const
{
    return get_property("mesh").get_value<std::shared_ptr<graphics::mesh>>();
}

void mesh_filter::set_mesh(std::shared_ptr<graphics::mesh> m)
{
    get_property("mesh").set_value(std::move(m));
}

template <>
void mesh_filter::serialize<json_serializer>(json_serializer& s)
{
    s.add_component(nlohmann::json {
        { "type", mesh_filter::type_name },
        { "is_enabled", is_enabled() },
    });
}

void mesh_filter::deserialize(const nlohmann::json& j)
{
    set_enabled(j[ "is_enabled" ]);
}
} // namespace components
