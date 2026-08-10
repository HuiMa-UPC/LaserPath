#pragma once

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QMouseEvent>

class TitleBar : public QWidget
{
	Q_OBJECT

public:
	TitleBar(QWidget *parent = nullptr);
	~TitleBar() override = default;

	void setMaxIcon(bool maximized);
signals:
	void minClicked();
	void maxClicked();
	void closeClicked();

private:

#pragma region 常量
	const int kHeight = 30;
	const int kWidthMargin = 5;
	const int kSpacing = 5;
#pragma endregion

#pragma region QSS
	const QString titleBarQSS = R"(
QWidget#TitleBar {
	background: #262E30;
}
)";

	const QString titleBtnQSS = R"(
QPushButton#TitleBtnQSS {
	background: transparent;
	border: 1px solid transparent;
	icon-size: 18px 18px;
	padding: 4px 4px;

}
QPushButton#TitleBtnQSS:hover {
	background: #003054;
	border: 1px solid #0078D4;
}
QPushButton#TitleBtnQSS:pressed {
    background: #00172A;
    border: 1px solid #0078D4;

}
)";
#pragma endregion

#pragma region 控件
	QPushButton* m_btnFile = nullptr;
	QPushButton* m_dropdownMenu = nullptr;

	QPushButton* m_btnSave = nullptr;
	QPushButton* m_btnUndo = nullptr;
	QPushButton* m_btnRedo = nullptr;

	QPushButton* m_btnMin = nullptr;
	QPushButton* m_btnMax = nullptr;
	QPushButton* btnClose = nullptr;
#pragma endregion

#pragma region 变量
	bool m_dragging = false;
	QPoint m_dragStartPos;
#pragma endregion

	void setTitleBarUI(QWidget* parent = nullptr);

protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
};

