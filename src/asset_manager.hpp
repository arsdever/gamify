#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "utils.hpp"

class mesh;
class material;
class image;
class shader_program;

class asset_manager
{
private:
    asset_manager() = default;

public:
    void load_asset(std::string_view path);
    template <typename T>
    void save_asset(std::string_view path, const T* asset);
    const std::vector<mesh*> meshes() const;
    const std::vector<material*> materials() const;
    const std::vector<image*> textures() const;
    const std::vector<shader_program*> shaders() const;

    mesh* get_mesh(std::string_view name) const;
    shader_program* get_shader(std::string_view name) const;
    material* get_material(std::string_view name) const;
    image* get_image(std::string_view name) const;

    static asset_manager* default_asset_manager();
    static void initialize();

private:
    static void initialize_quad_mesh();
    static void initialize_surface_shader();
    static std::string_view internal_resource_path();

private:
    template <typename T>
    using asset_map =
        std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

    asset_map<mesh*> _meshes;
    asset_map<material*> _materials;
    asset_map<image*> _textures;
    asset_map<shader_program*> _shaders;
    static asset_manager* _instance;
};
