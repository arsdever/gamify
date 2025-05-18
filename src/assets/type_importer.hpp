#pragma once

#include "assets/asset_cache.hpp"
#include "assets/asset_manager.hpp"
#include "common/file.hpp"
#include "common/filesystem.hpp"
#include "common/logging.hpp"

namespace assets
{
namespace
{
inline static logger log() { return get_logger("asset_manager"); }
} // namespace

class type_importer_base
{
public:
    virtual ~type_importer_base() = default;

    virtual void load_asset(asset& ast)
    {
        // Does nothing
    }
    virtual void update_asset(asset& ast)
    {
        // Does nothing
    }
    virtual float get_progress() const { return 0.0f; }
};

template <typename T>
class type_importer : public type_importer_base
{
public:
    using asset_data_t = std::shared_ptr<T>;

    void load_asset(asset& ast) override
    {
        initialize_asset(ast);
        _data = ast.template get_raw_data<T>();
        read_asset_data(ast.file_path());
    }

    void update_asset(asset& ast) override
    {
        _data = ast.template get_raw_data<T>();
        if (!_data)
        {
            load_asset(ast);
            return;
        }

        read_asset_data(ast.file_path());
    }

protected:
    virtual void initialize_asset(asset& ast) = 0;
    virtual void read_asset_data(std::string_view path) = 0;

protected:
    std::shared_ptr<T> _data;
};
} // namespace assets
