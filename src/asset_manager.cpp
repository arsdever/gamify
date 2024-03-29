#include <filesystem>

#include "asset_manager.hpp"

#include "asset_loaders/fbx.hpp"
#include "asset_loaders/jpg.hpp"
#include "asset_loaders/mat.hpp"
#include "asset_loaders/png.hpp"
#include "asset_loaders/shader.hpp"
#include "file.hpp"
#include "image.hpp"
#include "logging.hpp"
#include "mesh.hpp"
#include "shader.hpp"

namespace
{
static logger log() { return get_logger("asset_manager"); }
} // namespace

void asset_manager::load_asset(std::string_view path)
{
    auto [ _, filename, extension ] = parse_path(path);
#ifdef GAMIFY_SUPPORTS_FBX
    if (extension == ".fbx")
    {
        asset_loader_FBX fbx_loader;
        fbx_loader.load(path);
        auto [ it, success ] =
            _meshes.try_emplace(filename, fbx_loader.get_meshes());
        return;
    }
#endif
#ifdef GAMIFY_SUPPORTS_SHADER
    if (extension == ".shader")
    {
        asset_loader_SHADER shader_loader;
        shader_loader.load(path);
        auto [ it, success ] =
            _shaders.try_emplace(filename, shader_loader.get_shader_program());
        return;
    }
#endif
#ifdef GAMIFY_SUPPORTS_MAT
    if (extension == ".mat")
    {
        asset_loader_MAT mat_loader;
        mat_loader.load(path);
        auto [ it, success ] =
            _materials.try_emplace(filename, mat_loader.get_material());
        return;
    }
#endif
#ifdef GAMIFY_SUPPORTS_JPG
    if (extension == ".jpg" || extension == ".jpeg")
    {
        asset_loader_JPG jpg_loader;
        jpg_loader.load(path);
        auto [ it, success ] =
            _textures.try_emplace(filename, jpg_loader.get_image());
        return;
    }
#endif
#ifdef GAMIFY_SUPPORTS_PNG
    if (extension == ".png")
    {
        asset_loader_PNG png_loader;
        png_loader.load(path);
        auto [ it, success ] =
            _textures.try_emplace(filename, png_loader.get_image());
        return;
    }
#endif

    log()->error("Asset manager doesn't support {} format", extension);
}

template <>
void asset_manager::save_asset<image>(std::string_view path, const image* img)
{
    auto [ _, filename, extension ] = parse_path(path);
#ifdef GAMIFY_SUPPORTS_PNG
    if (extension == ".png")
    {
        asset_loader_PNG png_loader;
        png_loader.set_image(const_cast<image*>(img));
        png_loader.save(path);
        return;
    }
#endif
}

const std::vector<mesh*> asset_manager::meshes() const
{
    std::vector<mesh*> result;
    for (auto& [ _, value ] : _meshes)
    {
        result.insert(result.end(), value.begin(), value.end());
    }
    return result;
}

const std::vector<material*> asset_manager::materials() const
{
    std::vector<material*> result;
    for (auto& [ _, value ] : _materials)
    {
        result.push_back(value);
    }
    return result;
}

const std::vector<image*> asset_manager::textures() const
{
    std::vector<image*> result;
    for (auto& [ _, value ] : _textures)
    {
        result.push_back(value);
    }
    return result;
}

const std::vector<shader_program*> asset_manager::shaders() const
{
    std::vector<shader_program*> result;
    for (auto& [ _, value ] : _shaders)
    {
        result.push_back(value);
    }
    return result;
}

mesh* asset_manager::get_mesh(std::string_view name) const
{
    auto it = _meshes.find(name);
    return it == _meshes.end()
               ? nullptr
               : (it->second.empty() ? nullptr : it->second[ 0 ]);
}

shader_program* asset_manager::get_shader(std::string_view name) const
{
    return _shaders.contains(name) ? _shaders.find(name)->second : nullptr;
}

material* asset_manager::get_material(std::string_view name) const
{
    return _materials.contains(name) ? _materials.find(name)->second : nullptr;
}

image* asset_manager::get_image(std::string_view name) const
{
    return _textures.contains(name) ? _textures.find(name)->second : nullptr;
}

asset_manager* asset_manager::default_asset_manager()
{
    if (_instance == nullptr)
    {
        initialize();
    }

    return _instance;
}

void asset_manager::initialize()
{
    _instance = new asset_manager;
    initialize_quad_mesh();
    initialize_surface_shader();
}

void asset_manager::initialize_quad_mesh()
{
    auto quad_mesh = new mesh();
    std::array<glm::vec2, 4> verts {
        { { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 } }
    };

    std::vector<vertex3d> vertices;
    for (auto& v : verts)
    {
        vertex3d single_vertex;
        single_vertex.position() = { v.x, v.y, 0 };
        single_vertex.uv() = { v.x, v.y };
        vertices.push_back(std::move(single_vertex));
    }
    quad_mesh->set_vertices(std::move(vertices));
    quad_mesh->set_indices({ 0, 1, 2, 0, 2, 3 });
    quad_mesh->init();
    _instance->_meshes.try_emplace("quad", std::vector<mesh*> { quad_mesh });
}

void asset_manager::initialize_surface_shader()
{
    _instance->load_asset("surface.shader");
}

std::string_view asset_manager::internal_resource_path() { return ""; }

asset_manager* asset_manager::_instance = nullptr;
