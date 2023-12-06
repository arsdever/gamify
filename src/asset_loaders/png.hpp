#pragma once

#include "asset_loaders/asset_loader.hpp"

class image;

class asset_loader_PNG : public asset_loader
{
public:
    ~asset_loader_PNG() = default;
    void load(std::string_view path) override;

    image* get_image();

private:
    image* _image;
};