#pragma once

#include "components/collider_component.hpp"

class sphere_collider_component : public collider_component
{
public:
    sphere_collider_component(game_object* parent);

    void set_radius(float radius);
    float get_radius();

    void draw_gizmos() override;

    static constexpr std::string_view class_type_id =
        "sphere_collider_component";

protected:
    std::optional<collision> detect_collision(std::array<glm::vec3, 2> ray) override;

private:
    float _radius { 1.0f };
};
