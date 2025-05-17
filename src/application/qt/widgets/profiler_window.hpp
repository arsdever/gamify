#pragma once

#include <QMainWindow>

class ProfilerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProfilerWindow(QWidget* parent = nullptr);
    ~ProfilerWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    struct impl;
    std::unique_ptr<impl> _impl;
};
