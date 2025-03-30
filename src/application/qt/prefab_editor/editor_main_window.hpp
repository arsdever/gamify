#pragma once

#include <QMainWindow>

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    EditorMainWindow(QWidget* parent = nullptr);
    ~EditorMainWindow() override;

private:
    void initialize();
};
