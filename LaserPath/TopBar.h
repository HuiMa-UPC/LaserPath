#pragma once

#include <QWidget>
#include <QString>
#include <QPushButton>

class TopBar : public QWidget
{
	Q_OBJECT

public:
	TopBar(QWidget *parent = nullptr);
	~TopBar() override = default;

private:

#pragma region 常量
	const int kHeight = 35;
#pragma endregion

#pragma region QSS
	const QString topBarQSS = R"(
QWidget#TopBar {
	background: #3B4446;
}
)";
	const QString tapBtnQSS = R"(
QPushButton#TapBtnQSS {
	background: transparent;
	color: #FFFFFF;
	border: none;
	icon-size: 19px 19px;
	width: 135px;
	height: 35px;
}
QPushButton#TapBtnQSS:hover {
	background: #6B6B6B;
}
QPushButton#TapBtnQSS:checked {
	background: #00AE42;
}
)";
	const QString rightBtnQSS = R"(
QPushButton#RightBtnQSS {
	background: #00AE42;
	color: #FFFFFF;
	border: 1px solid #3B4446;
	width: 120px;
	height: 25px;
	border-radius:12px;
}
QPushButton#RightBtnQSS:hover {
	background: #30DD70;
}
QPushButton#RightBtnQSS:pressed {
	background: #17813F;
}
)";
#pragma endregion


	QPushButton* m_prepareBtn = nullptr;
	QPushButton* m_previewBtn = nullptr;
	QPushButton* m_sliceBtn = nullptr;
	QPushButton* m_printBtn = nullptr;
	void setTopBarUI();
};

