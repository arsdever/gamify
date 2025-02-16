#pragma once

#include "common/event.hpp"
#include "common/file_watcher.hpp"

namespace common
{
class directory
{
public:
    directory(std::string path);
    ~directory();

    void visit_files(std::function<void(std::string, bool)> cb);

private:
    struct impl;
    std::unique_ptr<impl> _impl;
    file_watcher _watcher;
};
} // namespace common
