#include "TopBar.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QIcon>

TopBar::TopBar(QWidget *parent)
	: QWidget(parent)
{
	setTopBarUI();
}

void TopBar::setTopBarUI()
{
	setFixedHeight(kHeight);
	setObjectName("TopBar");
	setStyleSheet(topBarQSS);
	setAttribute(Qt::WA_StyledBackground, true);

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->setAlignment(Qt::AlignCenter);

	auto addTapBtn = [&](const QString& icon, const QString& text) {
		QPushButton* btn = new QPushButton(this);
		btn->setIcon(QIcon(icon));
		btn->setObjectName("TapBtnQSS");
		btn->setStyleSheet(tapBtnQSS);
		btn->setCheckable(true);
		btn->setText(text);
		mainLayout->addWidget(btn);
		return btn;
		};

	auto addRightBtn = [&](const QString& text) {
		QPushButton* btn = new QPushButton(this);
		btn->setObjectName("RightBtnQSS");
		btn->setStyleSheet(rightBtnQSS);
		btn->setText(text);
		mainLayout->addWidget(btn);
		return btn;
		};

	m_prepareBtn = addTapBtn(":/icons/prepare", "  准备");
	m_prepareBtn->setChecked(true);
	m_previewBtn = addTapBtn(":/icons/preview", "  预览");

	connect(m_prepareBtn, &QPushButton::clicked, this, [this]() {
		m_prepareBtn->setChecked(true);
		m_previewBtn->setChecked(false);
		});
	connect(m_previewBtn, &QPushButton::clicked, this, [this]() {
		m_previewBtn->setChecked(true);
		m_prepareBtn->setChecked(false);
		});
	mainLayout->addStretch(1);
	m_sliceBtn = addRightBtn("生成路径");
	mainLayout->addSpacing(15);
	m_printBtn = addRightBtn("导出路径");
	mainLayout->addSpacing(40);
}



