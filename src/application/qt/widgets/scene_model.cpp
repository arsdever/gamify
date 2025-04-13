#include <project/game_object.hpp>
#include <project/scene.hpp>

#include "scene_model.hpp"

namespace ui
{
struct SceneModel::SceneModelPrivate
{
    std::weak_ptr<scene> _scene;
};

SceneModel::SceneModel(std::shared_ptr<scene> s, QObject* parent)
    : QAbstractItemModel(parent)
    , _p(std::make_unique<SceneModelPrivate>(s))
{
    scene::active_scene_changed += [ this ]
    {
        this->beginResetModel();
        setScene(scene::get_active_scene());
        this->endResetModel();
    };

    setScene(s);
}

SceneModel::~SceneModel() = default;

void SceneModel::setScene(std::shared_ptr<scene> s)
{
    if (_p->_scene = s; s == nullptr)
    {
        return;
    }

    s->root_object_added +=
        [ this ](size_t index, std::shared_ptr<game_object> obj)
    {
        this->beginInsertRows(QModelIndex(), index, index);
        this->endInsertRows();
    };
}

QModelIndex
SceneModel::index(int row, int column, const QModelIndex& parent) const
{
    if (auto sc = _p->_scene.lock(); sc != nullptr)
    {
        return createIndex(row, column);
    }

    return {};
}

QModelIndex SceneModel::parent(const QModelIndex& index) const
{
    if (index.column() > 0)
    {
        auto parent_game_object = data(parent(index), Qt::UserRole)
                                      .value<std::shared_ptr<game_object>>();

        if (parent_game_object == nullptr)
        {
            return {};
        }

        auto grandparent_game_object = parent_game_object->get_parent();

        if (grandparent_game_object == nullptr)
        {
            // This should be a root game object
            auto idx =
                _p->_scene.lock()->get_root_object_index(parent_game_object);
            return createIndex(idx, index.column());
        }

        auto idx = grandparent_game_object->get_child_index(parent_game_object);
        return createIndex(idx, index.column());
    }

    return {};
}

int SceneModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        if (auto sc = _p->_scene.lock(); sc != nullptr)
        {
            return sc->get_root_object_count();
        }

        return 0;
    }

    auto parent_game_object =
        data(parent, Qt::UserRole).value<std::shared_ptr<game_object>>();

    if (parent_game_object == nullptr)
    {
        return _p->_scene.lock()->get_root_object_count();
    }

    return parent_game_object->get_child_count();
}

int SceneModel::columnCount(const QModelIndex& parent) const { return 1; }

QVariant SceneModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // First determine whether or not this is a root object
    auto parent_index = parent(index);
    auto parent_game_object =
        data(parent_index, Qt::UserRole).value<std::shared_ptr<game_object>>();
    bool is_root = parent_game_object == nullptr;

    QVariant result;
    if (role == Qt::UserRole)
    {
        result.setValue(is_root
                            ? _p->_scene.lock()->get_root_object(index.row())
                            : parent_game_object->get_child_at(index.row()));
        return result;
    }

    auto associated_game_object =
        data(index, Qt::UserRole).value<std::shared_ptr<game_object>>();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        result.setValue(
            QString::fromStdString(associated_game_object->get_name()));
        return result;
    }
    default: return QVariant();
    }
}
} // namespace ui
