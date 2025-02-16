#pragma once

#include "assets/assets_fwd.hpp"

#include "assets/type_importer.hpp"

namespace assets
{
class material_importer : public assets::type_importer<graphics::material>
{
public:
    static constexpr std::string_view extensions { ".mat" };
    void initialize_asset(asset& ast) override;
    void read_asset_data(std::string_view asset_path) override;
};
} // namespace assets
