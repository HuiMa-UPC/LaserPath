#include "CustomDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>

const QString CustomDialog::kStyle = R"(
QDialog#CustomDialog {
	background: #FFFFFF;
}
QLabel#MsgLabel {
	font-size: 10pt;
	color: #333333;
	padding: 20px 20px 14px 20px;
}
QLineEdit#CInput {
	border: 1px solid #ABADAE;
	border-radius: 0px;
	min-height: 24px;
	max-height: 24px;
	padding: 0 8px;
	margin: 0 20px 14px 20px;
	font-size: 10pt;
}
QLineEdit#CInput:focus {
	border: 1px solid #00AE42;
}
QWidget#BtnBar {
	background: transparent;
	padding: 0 10px 0 10px;
}
QPushButton#CBtn {
	border: 1px solid #C0C0C0;
	border-radius: 12px;
	background: #FFFFFF;
	color: #000000;
	min-width: 30px;
	max-width: 30px;
	min-height: 24px;
	max-height: 24px;
	font-size: 10pt;
	padding: 0 16px;
}
QPushButton#CBtn:hover {
	border: 1px solid #C0C0C0;
	background: #C0C0C0;
}
QPushButton#CBtnPrimary {
	border: 1px solid #00AE42;
	border-radius: 12px;
	background: #00AE42;
	color: #FFFFFF;
	min-width: 30px;
	max-width: 30px;
	min-height: 24px;
	max-height: 24px;
	font-size: 10pt;
	padding: 0 16px;
}
QPushButton#CBtnPrimary:hover {
	background: #3DCB73;
	border: 1px solid #1B8844;
}
QPushButton#CBtnPrimary:pressed {
	background: #1B8844;
	border: 1px solid #1B8844;
}
)";

CustomDialog::CustomDialog(QWidget* parent, const QString& title)
	: QDialog(parent)
{
	setObjectName("CustomDialog");
	setFixedWidth(400);
	setFixedHeight(130);
	setStyleSheet(kStyle);
	setWindowTitle(title);
	setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::WindowStaysOnTopHint);
}

void CustomDialog::buildInputUi(const QString& label, const QString& initial)
{
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(10, 0, 10, 10);
	layout->setSpacing(0);

	auto* msgLabel = new QLabel(label, this);
	msgLabel->setObjectName("MsgLabel");
	layout->addWidget(msgLabel);

	m_input = new QLineEdit(initial, this);
	m_input->setObjectName("CInput");
	m_input->selectAll();
	layout->addWidget(m_input);

	auto* btnBar = new QWidget(this);
	btnBar->setObjectName("BtnBar");
	auto* btnLayout = new QHBoxLayout(btnBar);
	btnLayout->setContentsMargins(0, 0, 0, 0);
	btnLayout->setSpacing(10);
	btnLayout->addStretch();

	m_okBtn = new QPushButton(QStringLiteral("确定"), btnBar);
	m_okBtn->setObjectName("CBtnPrimary");
	m_okBtn->setDefault(true);
	btnLayout->addWidget(m_okBtn);
	btnLayout->addSpacing(5);

	m_cancelBtn = new QPushButton(QStringLiteral("取消"), btnBar);
	m_cancelBtn->setObjectName("CBtn");
	btnLayout->addWidget(m_cancelBtn);
	btnLayout->addSpacing(40);

	layout->addWidget(btnBar);

	connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
	connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void CustomDialog::buildConfirmUi(const QString& message)
{
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(10, 0, 10, 10);
	layout->setSpacing(0);

	auto* msgLabel = new QLabel(message, this);
	msgLabel->setObjectName("MsgLabel");
	msgLabel->setWordWrap(true);
	layout->addWidget(msgLabel);

	auto* btnBar = new QWidget(this);
	btnBar->setObjectName("BtnBar");
	auto* btnLayout = new QHBoxLayout(btnBar);
	btnLayout->setContentsMargins(0, 0, 0, 0);
	btnLayout->setSpacing(5);
	btnLayout->addStretch();

	m_okBtn = new QPushButton(QStringLiteral("是"), btnBar);
	m_okBtn->setObjectName("CBtnPrimary");
	m_okBtn->setDefault(true);
	btnLayout->addWidget(m_okBtn);
	btnLayout->addSpacing(5);

	m_cancelBtn = new QPushButton(QStringLiteral("否"), btnBar);
	m_cancelBtn->setObjectName("CBtn");
	btnLayout->addWidget(m_cancelBtn);
	btnLayout->addSpacing(40);

	layout->addWidget(btnBar);

	connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
	connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

QString CustomDialog::getText(QWidget* parent, const QString& title,
	const QString& label, const QString& initial, bool* ok)
{
	CustomDialog dlg(parent, title);
	dlg.buildInputUi(label, initial);
	if (dlg.exec() == QDialog::Accepted) {
		if (ok) *ok = true;
		return dlg.m_input->text();
	}
	if (ok) *ok = false;
	return QString();
}

bool CustomDialog::confirm(QWidget* parent, const QString& title,
	const QString& message)
{
	CustomDialog dlg(parent, title);
	dlg.buildConfirmUi(message);
	return dlg.exec() == QDialog::Accepted;
}
