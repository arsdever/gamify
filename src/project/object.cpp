#include <string>

#include "object.hpp"

uid generate_unique_id()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);

    static size_t id = 0;
    id = dis(gen) | dis(gen);
    id *= dis(gen);
    id *= dis(gen);
    id ^= dis(gen);

    return { id };
}

object::object()
    : _id(generate_unique_id())
{
}

object::~object() = default;

std::string object::get_name() const { return _name; }

void object::set_name(const std::string& name) { _name = name; }

uid object::id() const { return _id; }

std::unordered_map<size_t, std::weak_ptr<object>> object::_objects;
