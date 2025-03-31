#pragma once

#include "project/project_fwd.hpp"

class TransformInspector : public QWidget
{
    Q_OBJECT
public:
    TransformInspector(std::shared_ptr<components::transform> t,
                       QWidget* parent = nullptr);

private:
    std::shared_ptr<components::transform> _transform;
};
