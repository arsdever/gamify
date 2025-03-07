#include "application/application_commands.hpp"

#include "assets/asset_manager.hpp"
#include "common/logging.hpp"
#include "graphics/texture.hpp"
#include "tools/material_viewer/material_viewer.hpp"
#include "tools/mesh_viewer/mesh_viewer.hpp"
#include "tools/profiler/profiler.hpp"
#include "tools/texture_viewer/texture_viewer.hpp"

namespace
{
logger log() { return get_logger("console"); }
} // namespace

void cmd_show_profiler::execute()
{
    auto pv = std::make_shared<profiler>();
    pv->init();
    open_window_requested(pv);
}

void cmd_show_mesh::execute()
{
    auto mesh_asset = assets::asset_manager::try_get(get<0>());
    if (!mesh_asset)
    {
        log()->error("Mesh '{}' not found", get<0>());
        return;
    }

    auto mv = std::make_shared<mesh_viewer>();
    mv->init();
    log()->debug("Showing mesh '{}'", get<0>());
    mv->set_mesh(mesh_asset->as<graphics::mesh>());
    open_window_requested(mv);
}

void cmd_show_material::execute()
{
    auto material_asset = assets::asset_manager::try_get(get<0>());
    if (!material_asset)
    {
        log()->error("Material '{}' not found", get<0>());
        return;
    }

    auto mv = std::make_shared<material_viewer>();
    mv->init();
    log()->debug("Showing material '{}'", get<0>());
    mv->set_material(material_asset->as<graphics::material>());
    mv->set_mesh(
        assets::asset_manager::try_get<graphics::mesh>("meshes.sphere.fbx"));
    open_window_requested(mv);
}

void cmd_show_texture::execute()
{
    auto texture_asset =
        assets::asset_manager::try_get<graphics::texture>(get<0>());
    if (!texture_asset)
    {
        log()->error("Texture '{}' not found", get<0>());
        return;
    }

    auto tv = std::make_shared<texture_viewer>();
    tv->init();
    log()->debug("Showing texture '{}'", get<0>());
    tv->set_texture(texture_asset);
    open_window_requested(tv);
}

void cmd_list_textures::execute()
{
    for (int i = 0; i < graphics::texture::_textures.size(); ++i)
    {
        log()->info("Texture {}: id {}",
                    i,
                    graphics::texture::_textures[ i ]->native_id());
    }
}

event<void(std::shared_ptr<core::window>)> cmd_show_mesh::open_window_requested;
event<void(std::shared_ptr<core::window>)>
    cmd_show_material::open_window_requested;
event<void(std::shared_ptr<core::window>)>
    cmd_show_profiler::open_window_requested;
event<void(std::shared_ptr<core::window>)>
    cmd_show_texture::open_window_requested;
