#pragma once

#include "components/renderer_component.hpp"

class material;

class mesh_renderer_component : public renderer_component
{
public:
    mesh_renderer_component(game_object* parent);

    void render() override;

    static constexpr std::string_view class_type_id = "mesh_renderer_component";
};
