#pragma once

#include <QMainWindow>

class ProfilerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProfilerWindow(QWidget* parent = nullptr);
    ~ProfilerWindow() override = default;

private:
};
