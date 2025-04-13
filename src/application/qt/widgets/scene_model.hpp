#pragma once

#include <QAbstractItemModel>

class scene;

namespace ui
{
class SceneModel : public QAbstractItemModel
{
public:
    SceneModel(std::shared_ptr<scene> scene = nullptr,
               QObject* parent = nullptr);
    ~SceneModel();

    void setScene(std::shared_ptr<scene> scene);
    QModelIndex indexOf(std::shared_ptr<game_object> object);

    QModelIndex index(int row,
                      int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;

private:
    struct SceneModelPrivate;
    std::unique_ptr<SceneModelPrivate> _p;
};
} // namespace ui
