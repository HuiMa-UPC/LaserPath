#include "SectionHeader.h"
#include "Styles.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

SectionHeader::SectionHeader(const QString& iconPath,
    const QString& title,
    const QString& actionIconPath,
    QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName(QStringLiteral("SectionHeader"));
    setStyleSheet(sectionHeaderQSS + Styles::kIconBtn);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignVCenter);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(0);

    // 左侧图标
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(iconPath).pixmap(18, 18));

    // 分组标题
    QLabel* titleLabel = new QLabel(title, this);

    layout->addWidget(iconLabel);
    layout->addSpacing(5);
    layout->addWidget(titleLabel);
    layout->addStretch(1);

    if (!actionIconPath.isEmpty())
    {
        m_actionBtn = new QPushButton(this);
        m_actionBtn->setIcon(QIcon(actionIconPath));
        m_actionBtn->setObjectName("IconBtn");
        layout->addWidget(m_actionBtn);
    }
}

void SectionHeader::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QWidget* child = childAt(event->pos());

        if (!child || !qobject_cast<QPushButton*>(child))
        {
            m_pressed = true;
        }
    }

    QWidget::mousePressEvent(event);
}

void SectionHeader::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_pressed && rect().contains(event->pos()))
        {
            emit clicked();
        }
        m_pressed = false;
    }
    QWidget::mouseReleaseEvent(event);
}



