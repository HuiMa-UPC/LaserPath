#include "TitleBar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QLabel>

TitleBar::TitleBar(QWidget *parent)
	: QWidget(parent)
{
	setTitleBarUI(parent);
}

void TitleBar::setTitleBarUI(QWidget* parent)
{
    setFixedHeight(kHeight);
    setObjectName("TitleBar");
    setStyleSheet(titleBarQSS);
    setAttribute(Qt::WA_StyledBackground, true);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(kWidthMargin, 0, kWidthMargin, 0);
    mainLayout->setSpacing(kSpacing);
    mainLayout->setAlignment(Qt::AlignCenter);

    auto addBtn = [&](const QString& icon, const QString& tip) {
        QPushButton* btn = new QPushButton(this);
        btn->setIcon(QIcon(icon));
        btn->setObjectName("TitleBtnQSS");
        btn->setStyleSheet(titleBtnQSS);
        btn->setToolTip(tip);
        mainLayout->addWidget(btn);
        return btn;
        };

    m_btnFile = addBtn(":/icons/file", "File");
    m_dropdownMenu = addBtn(":/icons/menu", "Drop Down");

    QLabel* lineLabel = new QLabel(this);
    lineLabel->setPixmap(QIcon(":/icons/sep").pixmap(2, 20));
    mainLayout->addWidget(lineLabel);

    m_btnSave = addBtn(":/icons/save", "Save");
    m_btnUndo = addBtn(":/icons/undo", "Undo");
    m_btnRedo = addBtn(":/icons/redo", "Redo");

    mainLayout->addStretch(1);

    m_btnMin = addBtn(":/icons/min", "");
    mainLayout->addSpacing(5);
    m_btnMax = addBtn(":/icons/max", "");
    mainLayout->addSpacing(5);
    btnClose = addBtn(":/icons/close", "");

    connect(m_btnMin, &QPushButton::clicked, this, &TitleBar::minClicked);
    connect(m_btnMax, &QPushButton::clicked, this, &TitleBar::maxClicked);
    connect(btnClose, &QPushButton::clicked, this, &TitleBar::closeClicked);
}

void TitleBar::setMaxIcon(bool maximized)
{
    if (maximized)
        m_btnMax->setIcon(QIcon(":/icons/restore"));
    else 
        m_btnMax->setIcon(QIcon(":/icons/max"));
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget* child = childAt(event->pos());

    if (!child || !qobject_cast<QPushButton*>(child))
    {
        emit maxClicked();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (window()->isMaximized())
        {
            QWidget::mousePressEvent(event);
            return;
        }

        QWidget* child = childAt(event->pos());
        if (!child || !qobject_cast<QPushButton*>(child))
        {
            m_dragging = true;
            m_dragStartPos = event->globalPos() - window()->frameGeometry().topLeft();
        }
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging)
    {
        window()->move(event->globalPos() - m_dragStartPos);
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}







