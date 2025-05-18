#include "assets/importers/font_importer.hpp"

#include "graphics/font.hpp"

namespace assets
{
void font_importer::initialize_asset(asset& ast)
{
    ast.get_raw_data() = std::make_shared<graphics::font>();
}

void font_importer::read_asset_data(std::string_view asset_file)
{
    auto p = prof::profile(__PRETTY_FUNCTION__);
    _data->load(std::string(asset_file), 12.0f);
}
} // namespace assets
