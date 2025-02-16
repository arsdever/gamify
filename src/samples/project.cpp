#include "project/component_interface/component.hpp"
#include "project/component_interface/component_registry.hpp"
#include "project/components/transform.hpp"
#include "project/game_object.hpp"
#include "project/scene.hpp"

void create_scene_from_scratch()
{
    auto sc = scene::create();

    auto parent_object = game_object::create();
    auto object = game_object::create();

    parent_object->get_transform().set_rotation(
        glm::rotate(glm::identity<glm::dquat>(),
                    glm::radians(90.0),
                    glm::dvec3(1.0, 0.0, 0.0)));

    parent_object->get_transform().set_scale(glm::dvec3(2, 1, 1));

    object->get_transform().set_position(glm::dvec3(0.0, 0.0, 1.0));

    parent_object->set_name("Parent");
    object->set_name("Object");

    sc->add_root_object(parent_object);
    sc->add_root_object(object);

    // parent_object->set_name("Parent");
    // object->set_name("Object in space");
    parent_object->add_child(object);
    sc->save("scene.json");

    glm::mat4 world =
        object->get_transform()
            .get_matrix<components::transform::relation_flag::world>();

    std::cout
        << "\tLocal: "
        << glm::to_string(
               object->get_transform()
                   .get_matrix<components::transform::relation_flag::local>())
        << "\n\tWorld: " << glm::to_string(world) << "\n\tWorld position: "
        << glm::to_string(
               object->get_transform()
                   .get_position<components::transform::relation_flag::world>())
        << "\n\tWorld rotation: "
        << glm::to_string(
               object->get_transform()
                   .get_rotation<components::transform::relation_flag::world>())
        << "\n\tWorld scale: "
        << glm::to_string(
               object->get_transform()
                   .get_scale<components::transform::relation_flag::world>())
        << std::endl;
}

void load_scene()
{
    auto sc = scene::load("scene.json");

    // parent_object->set_name("Parent");
    // object->set_name("Object in space");
    sc->visit_root_objects([](const std::shared_ptr<game_object>& object)
    {
        glm::mat4 world =
            object->get_transform()
                .get_matrix<components::transform::relation_flag::world>();

        std::cout << "\tLocal: "
                  << glm::to_string(
                         object->get_transform()
                             .get_matrix<
                                 components::transform::relation_flag::local>())
                  << "\n\tWorld: " << glm::to_string(world)
                  << "\n\tWorld position: "
                  << glm::to_string(
                         object->get_transform()
                             .get_position<
                                 components::transform::relation_flag::world>())
                  << "\n\tWorld rotation: "
                  << glm::to_string(
                         object->get_transform()
                             .get_rotation<
                                 components::transform::relation_flag::world>())
                  << "\n\tWorld scale: "
                  << glm::to_string(
                         object->get_transform()
                             .get_scale<
                                 components::transform::relation_flag::world>())
                  << std::endl;
    });

    sc->visit_root_objects([](std::shared_ptr<game_object>& object)
    { object->init(); });
    while (1)
    {
        sc->visit_root_objects([](std::shared_ptr<game_object>& object)
        { object->init(); });
    }
    sc->visit_root_objects([](std::shared_ptr<game_object>& object)
    { object->init(); });
}

int main(int argc, char** argv)
{
    std::string mode = "r";
    std::cin >> mode;

    if (mode == "w")
    {
        create_scene_from_scratch();
    }
    else
    {
        load_scene();
    }

    return 0;
}
