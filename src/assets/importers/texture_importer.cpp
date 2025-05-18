#include "assets/importers/texture_importer.hpp"

#include "common/main_thread_dispatcher.hpp"
#include "graphics/texture.hpp"

namespace assets
{
void texture_importer::initialize_asset(asset& ast)
{
    ast.get_raw_data() = std::make_shared<graphics::texture>();
}

void texture_importer::read_asset_data(std::string_view asset_path)
{
    auto p = prof::profile(__PRETTY_FUNCTION__);
    _data->set_contents_from_file(asset_path);
}
} // namespace assets
