#pragma once

#include <graphics/graphics_fwd.hpp>

class QWidget;
class material_viewer;

class material_property_ui_builder
{
public:
    static QWidget* build(std::shared_ptr<material_viewer> viewer);

private:
    struct material_property_ui_builder_impl;
    static std::unique_ptr<material_property_ui_builder_impl> _impl;
};
