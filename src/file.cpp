#include <cstring>
#include <filesystem>

#include "file.hpp"

#include "logging.hpp"

namespace
{
static inline logger log() { return get_logger("fs"); }
} // namespace

std::string get_file_contents(std::string_view path)
{
    std::FILE* f;
    auto error = fopen_s(&f, path.data(), "r");

    if (error != 0)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        log()->error(
            "Failed to load the file at {}: {}", path, std::strerror(error));
        return "";
#pragma clang diagnostic pop
    }

    std::fseek(f, 0, SEEK_END);
    int size = std::ftell(f);

    std::string result(size, '\0');
    std::fseek(f, 0, SEEK_SET);
    std::fread(result.data(), 1, size, f);

    return result;
}

bool file_exists(std::string_view path)
{
    return std::filesystem::is_regular_file(path);
}

std::tuple<std::string, std::string, std::string>
parse_path(std::string_view path)
{
    std::filesystem::path p { path };
    return { p.root_directory().generic_string(),
             p.stem().generic_string(),
             p.extension().generic_string() };
}
