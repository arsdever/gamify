#include <nlohmann/json.hpp>

#include "assets/asset_manager.hpp"

#include "assets/asset.hpp"
#include "assets/asset_cache.hpp"
#include "assets/asset_importer.hpp"
#include "assets/importers/font_importer.hpp"
#include "assets/importers/material_importer.hpp"
#include "assets/importers/model_importer.hpp"
#include "assets/importers/script_importer.hpp"
#include "assets/importers/shader_importer.hpp"
#include "assets/importers/texture_importer.hpp"
#include "common/directory.hpp"
#include "common/filesystem.hpp"
#include "common/logging.hpp"
#include "common/main_thread_dispatcher.hpp"

namespace assets
{
struct asset_manager::impl
{
    std::string project_path;
    common::file_watcher directory_watcher;

    asset_cache _cache;
    asset_importer _importer;
    std::queue<std::string> _loading_queue;
    nlohmann::json _index { { "id_path", nlohmann::json {} },
                            { "key_id", nlohmann::json {} },
                            { "requires", nlohmann::json {} },
                            { "no_lazy_load", nlohmann::json {} } };

    void scan_directory()
    {
        load_index_file();
        update_index();
        save_index();

        resolve_dependencies();
        resolve_no_lazy_load();
    }

    void load_index_file()
    {
        auto project_path = common::filesystem::path(_impl->project_path);
        common::file index_file(
            std::string((project_path / "resources.json").full_path()));

        if (index_file.exists())
        {
            auto data = index_file.read_all();
            _index = nlohmann::json::parse(data);
            index_file.close();
        }
    }

    void update_index()
    {
        auto project_path = common::filesystem::path(_impl->project_path);
        common::file index_file(
            std::string((project_path / "resources.json").full_path()));

        std::function<void(common::filesystem::path)> scan_dir;
        scan_dir = [ & ](common::filesystem::path path)
        {
            common::directory dir(std::string(path.full_path()));
            dir.visit_files(
                [ this, scan_dir, path ](std::string file_path, bool is_dir)
            {
                if (is_dir)
                {
                    if (file_path == "." || file_path == "..")
                    {
                        return;
                    }

                    scan_dir(path / file_path);
                }
                else
                {
                    auto asset_path = path / file_path;
                    auto asset_key =
                        get_asset_key_by_path(asset_path.full_path());

                    if (!_index[ "key_id" ].contains(asset_key))
                    {
                        auto id = generate_random<size_t>();
                        _index[ "key_id" ][ asset_key ] = std::to_string(id);
                        _index[ "id_path" ][ std::to_string(id) ] =
                            path.full_path();
                    }

                    auto asset_load = [ &importer = _importer ](asset& ast)
                    { importer.load_asset(ast); };

                    auto ast = std::make_shared<asset>(
                        std::string { asset_path.full_path() }, asset_load);
                    _cache.register_asset(
                        std::stoull(
                            _index[ "key_id" ][ asset_key ].get<std::string>()),
                        std::move(ast));
                }
            });
        };
        scan_dir(project_path);
    }

    void save_index()
    {
        auto project_path = common::filesystem::path(_impl->project_path);
        common::file index_file(
            std::string((project_path / "resources.json").full_path()));

        common::file::remove(index_file.get_filepath());
        common::file::create(index_file.get_filepath(), _index.dump(2));
    }

    std::string get_resource_path_by_id(std::string id)
    {
        return _index[ "id_path" ][ id ];
    }

    void resolve_dependencies()
    {
        for (const auto& [ base, deps ] : _index[ "requires" ].items())
        {
            for (const auto& dep : deps)
            {
                auto ast =
                    _impl->_cache.find(std::stoull(dep.get<std::string>()));
                if (ast)
                {
                    ast->_on_modified += [ this, base ]()
                    {
                        auto base_ast = _impl->_cache.find(std::stoull(base));
                        if (!base_ast)
                        {
                            return;
                        }

                        if (base_ast->is_loaded())
                        {
                            _importer.update_asset(*base_ast);
                        }
                    };
                }
            }
        }
    }

    void resolve_no_lazy_load()
    {
        for (const auto& it : _index[ "no_lazy_load" ])
        {
            auto ast = _impl->_cache.find(std::stoull(it.get<std::string>()));
            if (!ast)
            {
                log()->warn("Invalid entry in no_lazy_load list: {}",
                            it.get<std::string>());
                continue;
            }

            load_asset(*ast);
        }
    }

    void setup_directory_watch()
    {
        try
        {
            directory_watcher = common::file_watcher(
                project_path,
                [ this ](std::string_view path, common::file_change_type change)
            {
                std::string path_str { (common::filesystem::path(
                                            _impl->project_path) /
                                        common::filesystem::path(path))
                                           .full_path() };
                common::main_thread_dispatcher::dispatch(
                    [ this, path = std::move(path_str), change ]
                {
                    log()->info("File {} changed: {}",
                                path,
                                static_cast<unsigned>(change));
                    switch (change)
                    {
                    case common::file_change_type::created:
                    case common::file_change_type::modified:
                    {
                        auto ast =
                            _impl->_cache.find(get_asset_key_by_path(path));
                        if (ast)
                        {
                            update_asset(*ast);
                        }
                    }
                    // case common::file_change_type::removed:
                    // unload_asset(path); break;
                    default: break;
                    }
                });
            });
        }
        catch (std::exception& e)
        {
            log()->error("Failed to setup directory watcher: {}", e.what());
            return;
        }
    }

    std::shared_ptr<asset> try_get_internal(std::string_view name)
    {
        return _impl->_cache.find(name);
    }

    std::string_view internal_resource_path() { return ""; }

    void load_asset(asset& ast)
    {
        _importer.load_asset(ast);
        ast._on_modified();
    }

    void update_asset(asset& ast)
    {
        _importer.update_asset(ast);
        ast._on_modified();
    }

    void apply_assets(
        std::function<void(std::string_view, std::shared_ptr<asset>)> func)
    {
        _cache.apply_assets([ func ](size_t id, std::shared_ptr<asset> ast)
        {
            std::string key = get_asset_key_by_id(id);
            func(key, ast);
        });
    }

    void perform_asset_load(asset& ast) { }

    void perform_asset_update(asset& ast) { }

    void perform_asset_unload(asset& ast) { }
};

void asset_manager::initialize(asset_manager* existing_instance)
{
    if (_impl)
        return;

    _impl = existing_instance->_impl;
}

void asset_manager::initialize(std::string_view resource_path)
{
    _impl = std::make_shared<impl>();
    _impl->project_path = common::filesystem::path(resource_path).full_path();

    register_importer(font_importer::extensions,
                      std::make_shared<font_importer>());
    register_importer(texture_importer::extensions,
                      std::make_shared<texture_importer>());
    register_importer(material_importer::extensions,
                      std::make_shared<material_importer>());
    register_importer(script_importer::extensions,
                      std::make_shared<script_importer>());
    register_importer(shader_importer::extensions,
                      std::make_shared<shader_importer>());
    register_importer(model_importer::extensions,
                      std::make_shared<model_importer>());
}

void asset_manager::shutdown()
{
    if (!_impl)
        return;

    _impl = nullptr;
}

void asset_manager::scan_project_directory() { _impl->scan_directory(); }

void asset_manager::setup_project_directory_watch()
{
    _impl->setup_directory_watch();
}

asset& asset_manager::get(std::string_view name) { return *try_get(name); }

std::shared_ptr<asset> asset_manager::try_get_internal(std::string_view name)
{
    return _impl->try_get_internal(name);
}

void asset_manager::apply_assets(
    std::function<void(std::string_view, std::shared_ptr<asset>)> func)
{
    _impl->apply_assets(func);
}

void asset_manager::register_importer(
    std::string_view key, std::shared_ptr<type_importer_base> importer)
{
    _impl->_importer.register_importer(key, importer);
}

asset_importer& asset_manager::get_importer() { return _impl->_importer; }

asset_cache& asset_manager::get_cache() { return _impl->_cache; }

size_t asset_manager::get_asset_id_by_path(std::string_view path)
{
    return get_asset_id_by_key(get_asset_key_by_path(path));
}

size_t asset_manager::get_asset_id_by_key(std::string_view key)
{
    if (!_impl->_index[ "key_id" ].contains(key))
    {
        return 0;
    }
    return std::stoull(_impl->_index[ "key_id" ][ key ].get<std::string>());
}

std::string asset_manager::get_asset_key_by_path(std::string_view path)
{
    auto pos = path.find(_impl->project_path);
    auto relpath = path.substr(
        pos != std::string::npos ? pos + _impl->project_path.size() + 1 : 0);
    std::string result = std::string(relpath);
    std::replace(result.begin(), result.end(), '/', '.');
    return std::string(result);
}

std::string asset_manager::get_asset_key_by_id(size_t id)
{
    return _impl->_index[ "id_path" ][ std::to_string(id) ];
}

std::shared_ptr<asset_manager::impl> asset_manager::_impl = nullptr;
} // namespace assets
