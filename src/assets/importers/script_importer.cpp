
#include "assets/importers/script_importer.hpp"

#include "scripting/backend.hpp"
#include "scripting/script.hpp"

namespace assets
{
void script_importer::initialize_asset(asset& ast)
{
    ast.get_raw_data() = std::make_shared<scripting::script>();
}

void script_importer::read_asset_data(std::string_view asset_path)
{
    auto p = prof::profile(__PRETTY_FUNCTION__);
    scripting::backend::load_script(asset_path);
}
} // namespace assets
