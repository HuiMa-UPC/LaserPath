#include "ParamHeader.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

ParamHeader::ParamHeader(const QString& icon, const QString& name, QWidget* parent)
	: QWidget(parent)
{
	setObjectName("ParamHeader");
	setStyleSheet(paramHeaderQSS);
	setAttribute(Qt::WA_StyledBackground, true);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	QLabel* iconLabel = new QLabel(this);
	iconLabel->setPixmap(QIcon(icon).pixmap(18, 18));
	QLabel* nameLabel = new QLabel(name, this);
	nameLabel->setObjectName("NameLabel");

	auto* line = new QWidget(this);
	line->setObjectName("Line");

	layout->addWidget(iconLabel);
	layout->addSpacing(5);
	layout->addWidget(nameLabel);
	layout->addSpacing(5);
	layout->addWidget(line, 1);
	layout->addSpacing(10);
}

