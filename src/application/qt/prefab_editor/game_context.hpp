#pragma once

class game_context
{
public:
    static void set_viewport_size(glm::uvec2 size);
    static void initialize();
    static void load_assets();
    static void render();

    static void init();
    static void update();
    static void deinit();

    static void create_empty_game_object();
    static void create_camera();
    static void create_cube();
};
