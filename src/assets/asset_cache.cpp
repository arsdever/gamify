#include "assets/asset_cache.hpp"

#include "assets/asset_manager.hpp"

namespace assets
{
void asset_cache::register_asset(std::string_view name,
                                 std::shared_ptr<asset> ast)
{
    register_asset(asset_manager::get_asset_id_by_key(name), ast);
}

void asset_cache::register_asset(size_t id, std::shared_ptr<asset> ast)
{
    _assets[ id ] = ast;
}

bool asset_cache::contains(std::string_view name)
{
    return contains(asset_manager::get_asset_id_by_key(name));
}

bool asset_cache::contains(size_t id)
{
    return _assets.find(id) != _assets.end();
}

std::shared_ptr<asset> asset_cache::find(size_t i)
{
    auto it = _assets.find(i);
    return it != _assets.end() ? it->second : nullptr;
}

std::shared_ptr<asset> asset_cache::find(std::string_view name)
{
    return find(asset_manager::get_asset_id_by_key(name));
}

void asset_cache::apply_assets(
    std::function<void(size_t, std::shared_ptr<asset>)> func)
{
    for (auto& ast : _assets)
    {
        func(ast.first, ast.second);
    }
}
} // namespace assets
