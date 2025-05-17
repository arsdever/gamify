#pragma once

#include <nlohmann/json_fwd.hpp>

#include "graphics/graphics_fwd.hpp"
#include "project/project_fwd.hpp"

#include "project/component_interface/component.hpp"

namespace components
{
class mesh_filter : public component
{
public:
    using base = component;

    mesh_filter(game_object& obj);

    static constexpr std::string_view type_name = "mesh_filter";

    std::shared_ptr<graphics::mesh> get_mesh() const;
    void set_mesh(std::shared_ptr<graphics::mesh> m);

    template <typename ST>
    void serialize(ST& s);
    void deserialize(const nlohmann::json& j);
};
} // namespace components
