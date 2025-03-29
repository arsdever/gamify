#pragma once

#include <nlohmann/json_fwd.hpp>

#include "graphics/graphics_fwd.hpp"
#include "project/project_fwd.hpp"

#include "project/component_interface/component.hpp"

namespace components
{
class mesh_renderer : public component
{
public:
    using base = component;

    mesh_renderer(game_object& obj);

    static constexpr std::string_view type_name = "mesh_renderer";

    std::vector<std::shared_ptr<graphics::material>> get_materials() const;
    void set_materials(std::vector<std::shared_ptr<graphics::material>> m);

    std::shared_ptr<graphics::material> get_material(size_t index) const;
    void add_material(std::shared_ptr<graphics::material> m);
    void set_material(size_t index, std::shared_ptr<graphics::material> m);

    template <typename ST>
    void serialize(ST& s);
    void deserialize(const nlohmann::json& j);

private:
    std::vector<std::shared_ptr<graphics::material>> _materials;
};
} // namespace components
