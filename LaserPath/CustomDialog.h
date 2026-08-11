#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class CustomDialog : public QDialog
{
	Q_OBJECT

public:
	enum Result { Accepted, Rejected };

	/// 输入弹窗（新建/编辑名称）
	static QString getText(QWidget* parent, const QString& title,
		const QString& label, const QString& initial = QString(),
		bool* ok = nullptr);

	/// 确认弹窗（删除确认等）
	static bool confirm(QWidget* parent, const QString& title,
		const QString& message);

private:
	CustomDialog(QWidget* parent, const QString& title);

	void buildInputUi(const QString& label, const QString& initial);
	void buildConfirmUi(const QString& message);

	QLineEdit* m_input = nullptr;
	QPushButton* m_okBtn = nullptr;
	QPushButton* m_cancelBtn = nullptr;

	static const QString kStyle;
};
