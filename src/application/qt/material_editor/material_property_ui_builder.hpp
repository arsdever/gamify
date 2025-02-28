#pragma once

#include <graphics/graphics_fwd.hpp>

class QWidget;

class material_property_ui_builder
{
public:
    static QWidget* build(std::shared_ptr<graphics::material> mat);
};
