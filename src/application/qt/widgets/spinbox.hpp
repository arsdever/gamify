#pragma once

#include <QAbstractSpinBox>

class QEvent;
class QEnterEvent;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

namespace ui
{
class SpinBox : public QWidget
{
    Q_OBJECT
public:
    SpinBox(QWidget* parent = nullptr);
    virtual ~SpinBox();

    void setLabel(const QString& label);
    QString label() const;

    void setValue(double value);
    double value() const;

    void setRange(double min, double max);
    double min() const;
    double max() const;

    void setSingleStep(double step);
    double singleStep() const;

    void setStep(double step);
    double step() const;

    void setPageStep(double step);
    double pageStep() const;

    void setDecimals(int decimals);
    int decimals() const;

    void setAccelerated(bool accelerated);
    bool isAccelerated() const;

signals:
    void valueChanged(double value);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct SpinBoxPrivate;
    std::unique_ptr<SpinBoxPrivate> _p;
};
} // namespace ui
