#include <cstring>

#include "file.hpp"

#include "logging.hpp"

namespace
{
static inline logger log() { return get_logger("fs"); }
} // namespace

void file::open(std::string_view path, std::string_view privileges)
{
    if (_state != state::closed)
    {
        log()->warn("Another file is already open. Closing");
        close();
    }

#ifdef WIN32
    auto error = fopen_s(&_file_descriptor, path.data(), privileges.data());
#else
    auto _file_descriptor = fopen(path.data(), privileges.data());
    int error;
    if (_file_descriptor)
    {
        error = errno;
    }
#endif

    if (error != 0)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        log()->error(
            "Failed to load the file at {}: {}", path, std::strerror(error));
        return;
#pragma clang diagnostic pop
    }
    _state = state::open;
}

size_t file::size() const
{
    if (_state != state::open)
    {
        log()->error("Can't get the size of a closed file");
        return -1;
    }

    int pos = std::ftell(_file_descriptor);
    std::fseek(_file_descriptor, 0, SEEK_END);
    int size = std::ftell(_file_descriptor);
    std::fseek(_file_descriptor, pos, SEEK_SET);
    return size;
}

std::string file::read_all()
{
    if (_state != state::open)
    {
        log()->error("Can't read the contents of a closed file");
        return "";
    }
    size_t length = size();
    std::string result(length, '\0');
    std::fseek(_file_descriptor, 0, SEEK_SET);
    std::fread(result.data(), 1, length, _file_descriptor);

    return result;
}
