#pragma once

#include <QWidget>

class ProfilerWidget : public QWidget
{
    Q_OBJECT
public:
    static ProfilerWidget* create(QWidget* parent = nullptr);

private:
    ProfilerWidget(QWidget* parent = nullptr);
    ~ProfilerWidget() override = default;

    struct impl;
    std::unique_ptr<impl> _p;
};
