#include "assets/asset.hpp"

#include "assets/asset_manager.hpp"

namespace assets
{
asset::asset(std::string file_path, std::function<void(asset&)> loader)
    : _file_path(std::move(file_path))
    , _loader(loader)
{
}

std::string asset::file_path() const { return _file_path; }

size_t asset::id() const { return asset_manager::get_asset_id_by_key(key()); }

std::string asset::key() const
{
    return asset_manager::get_asset_key_by_path(file_path());
}

void asset::ensure_data()
{
    if (!is_loaded())
    {
        _loader(const_cast<asset&>(*this));
    }
}

std::optional<std::any>& asset::get_raw_data() { return _data; }

bool asset::is_of_type(size_t type_index)
{
    return _data.has_value() && (type_index == _data->type().hash_code());
}

bool asset::is_loaded() const { return _data.has_value(); }
} // namespace assets
