#pragma once

#include <QMainWindow>

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorMainWindow(QWidget* parent = nullptr);
    ~EditorMainWindow() override;

private:
    struct impl;
    std::unique_ptr<impl> _p;
};
