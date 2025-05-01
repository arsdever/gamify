#pragma once

#include <QWidget>

#include <glm/fwd.hpp>

class ProfilerWidget : public QWidget
{
    Q_OBJECT
public:
    static ProfilerWidget* create(QWidget* parent = nullptr);

    glm::dvec2 zoom() const;
    glm::dvec2 scroll() const;

    void reset();

public slots:
    void setZoom(glm::dvec2 z);
    void setScroll(glm::dvec2 s);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    ProfilerWidget(QWidget* parent = nullptr);
    ~ProfilerWidget() override = default;

    struct impl;
    std::unique_ptr<impl> _p;
};
