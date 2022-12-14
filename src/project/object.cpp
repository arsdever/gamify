#include "project/object.hpp"

namespace g::project
{

object::object() = default;

object_ptr object::create(std::string_view name, object_ptr parent)
{
    std::shared_ptr<object> obj { new object() };
    obj->_parent = parent;
    obj->_name = name;

    if (parent)
        parent->add_child(obj);

    return obj;
}

object::~object() = default;

std::string object::name() { return _name; }

std::list<object_ptr> const& object::children() const { return _children; }

object_ptr object::parent() const { return _parent.lock(); }

object_ptr object::add_child(object_ptr obj)
{
    if (std::find(_children.begin(), _children.end(), obj) == _children.end())
    {
        // As the parent is being changed this child should be removed from the
        // old parent.
        if (object_ptr parent = obj->_parent.lock())
        {
            parent->remove_child(obj);
        }

        on_before_child_added(shared_from_this(), obj);
        obj->_parent = weak_from_this();
        _children.push_back(obj);
        on_children_list_changed();
        obj->on_parent_changed(obj->_parent.lock());
        on_child_added(shared_from_this(), obj);
    }
    return obj;
}

void object::remove_child(object_ptr obj)
{
    on_before_child_removed(shared_from_this(), obj);
    _children.remove(obj);
    obj->_parent.reset();
    on_children_list_changed();
    obj->on_parent_changed(shared_from_this());
    on_child_removed(shared_from_this(), obj);
}

void object::move(object_ptr parent)
{
    if (parent == _parent.lock())
        return;

    if (_parent.lock())
        _parent.lock()->remove_child(shared_from_this());

    _parent = parent;
    parent->add_child(shared_from_this());
}

object_cptr object::get_ptr() const { return shared_from_this(); }

object_ptr object::get_ptr() { return shared_from_this(); }

} // namespace g::project
