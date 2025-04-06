#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

#include "collapsible_widget.hpp"

namespace ui
{
struct CollapsibleWidget::CollapsibleWidgetPrivate
{
    QGridLayout* _mainLayout;
    QToolButton* _toggleButton;
    QFrame* _headerLine;
    QParallelAnimationGroup* _toggleAnimation;
    QScrollArea* _contentArea;
    int _animationDuration { 300 };
};

CollapsibleWidget::CollapsibleWidget(const QString& title,
                                     const int animationDuration,
                                     QWidget* parent)
    : QWidget(parent)
    , _p(std::make_unique<CollapsibleWidgetPrivate>())
{
    _p->_mainLayout = new QGridLayout(this);
    _p->_toggleButton = new QToolButton(this);
    _p->_headerLine = new QFrame(this);
    _p->_toggleAnimation = new QParallelAnimationGroup(this);
    _p->_contentArea = new QScrollArea(this);

    _p->_toggleButton->setStyleSheet("QToolButton { border: none; }");
    _p->_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _p->_toggleButton->setArrowType(Qt::ArrowType::RightArrow);
    _p->_toggleButton->setText(title);
    _p->_toggleButton->setCheckable(true);
    _p->_toggleButton->setChecked(false);

    _p->_headerLine->setFrameShape(QFrame::HLine);
    _p->_headerLine->setFrameShadow(QFrame::Sunken);
    _p->_headerLine->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Maximum);

    _p->_contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // start out collapsed
    _p->_contentArea->setMaximumHeight(0);
    _p->_contentArea->setMinimumHeight(0);
    // let the entire widget grow and shrink with its content
    _p->_toggleAnimation->addAnimation(
        new QPropertyAnimation(this, "minimumHeight"));
    _p->_toggleAnimation->addAnimation(
        new QPropertyAnimation(this, "maximumHeight"));
    _p->_toggleAnimation->addAnimation(
        new QPropertyAnimation(_p->_contentArea, "maximumHeight"));
    // don't waste space
    _p->_mainLayout->setVerticalSpacing(0);
    _p->_mainLayout->setContentsMargins(0, 0, 0, 0);
    int row = 0;
    _p->_mainLayout->addWidget(_p->_toggleButton, row, 0, 1, 1, Qt::AlignLeft);
    _p->_mainLayout->addWidget(_p->_headerLine, row++, 2, 1, 1);
    _p->_mainLayout->addWidget(_p->_contentArea, row, 0, 1, 3);
    setLayout(_p->_mainLayout);
    QObject::connect(_p->_toggleButton,
                     &QToolButton::clicked,
                     [ this ](const bool checked)
    {
        _p->_toggleButton->setArrowType(checked ? Qt::ArrowType::DownArrow
                                                : Qt::ArrowType::RightArrow);
        _p->_toggleAnimation->setDirection(checked
                                               ? QAbstractAnimation::Forward
                                               : QAbstractAnimation::Backward);
        _p->_toggleAnimation->start();
    });
}

CollapsibleWidget::~CollapsibleWidget() = default;

void CollapsibleWidget::setContentLayout(QLayout* contentLayout)
{
    if (_p->_contentArea->layout())
    {
        _p->_contentArea->layout()->deleteLater();
    }

    _p->_contentArea->setLayout(contentLayout);
    const auto collapsedHeight =
        sizeHint().height() - _p->_contentArea->maximumHeight();
    auto contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < _p->_toggleAnimation->animationCount() - 1; ++i)
    {
        QPropertyAnimation* spoilerAnimation = static_cast<QPropertyAnimation*>(
            _p->_toggleAnimation->animationAt(i));
        spoilerAnimation->setDuration(_p->_animationDuration);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }
    QPropertyAnimation* contentAnimation =
        static_cast<QPropertyAnimation*>(_p->_toggleAnimation->animationAt(
            _p->_toggleAnimation->animationCount() - 1));
    contentAnimation->setDuration(_p->_animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}
} // namespace ui
