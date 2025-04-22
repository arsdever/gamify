#pragma once

#include <pybind11/pybind11.h>

#include "common/logging.hpp"
#include "project/component_interface/component.hpp"

namespace scripting::python
{
class python_component : public component
{
public:
    using base = component;

    python_component(std::string_view name, game_object& obj);
    void on_init() override;
    void on_update() override;
    void on_deinit() override;

    pybind11::object _instance;
};

namespace internal
{
class python_component_trampoline : public python_component
{
public:
    using base = python_component;
    using base::base;

    void on_init() override;

    void on_update() override;

    void on_deinit() override;
};
} // namespace internal
} // namespace scripting::python
