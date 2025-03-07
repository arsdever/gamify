#pragma once

#include "assets/assets_fwd.hpp"
#include "graphics/graphics_fwd.hpp"

#include "assets/asset.hpp"
#include "assets/asset_importer.hpp"
#include "common/utils.hpp"

namespace assets
{
class asset_manager
{
private:
    asset_manager() = default;

public:
    static void initialize(asset_manager* existing_instance);
    static void initialize(std::string_view resource_path);
    static void shutdown();

    static void scan_project_directory();
    static void setup_project_directory_watch();

    template <typename T>
    static void save_asset(std::string_view path, const T* asset);
    static void register_asset(std::string_view name,
                               std::shared_ptr<asset> ast);

    static asset& get(std::string_view name);
    template <typename T>
    static auto get(std::string_view name)
    {
        return try_get<T>(name);
    }
    static auto try_get(std::string_view name)
    {
        return try_get_internal(name);
    }
    template <typename T>
    static std::shared_ptr<T> try_get(std::string_view name)
    {
        auto ast = try_get_internal(name);
        if (!ast)
            return nullptr;

        return ast->template as<T>();
    }

    template <typename T>
    static void apply(std::function<void(std::string_view, const T&)> func)
    {
        apply_asset([ func ](std::string_view name, std::shared_ptr<asset> ast)
        { func(name, ast->template as<T>()); });
    }

    template <typename T>
    static void
    apply(std::function<void(std::string_view, std::shared_ptr<asset>)> func)
    {
        apply_assets([ func ](std::string_view name, std::shared_ptr<asset> ast)
        {
            if (ast->is_of_type<std::shared_ptr<T>>())
                func(name, ast);
        });
    }

    static void apply_assets(
        std::function<void(std::string_view, std::shared_ptr<asset>)> func);

    static void register_importer(std::string_view key,
                                  std::shared_ptr<type_importer_base> importer);

    static asset_importer& get_importer();
    static asset_cache& get_cache();

    static size_t get_asset_id_by_path(std::string_view path);
    static size_t get_asset_id_by_key(std::string_view key);
    static std::string get_asset_key_by_path(std::string_view path);
    static std::string get_asset_key_by_id(size_t id);

private:
    static std::shared_ptr<asset> try_get_internal(std::string_view name);

private:
    struct impl;
    static std::shared_ptr<impl> _impl;
};
} // namespace assets
