#pragma once

#include <QWidget>
#include <QString>

class ParamHeader : public QWidget
{
	Q_OBJECT

public:
	ParamHeader(const QString& icon, const QString& name, QWidget* parent);
	~ParamHeader() override = default;

private:

#pragma region QSS
	const QString paramHeaderQSS = R"(
QWidget#ParamHeader {
	min-height: 20px;
	max-height: 20px;
}
QLabel#NameLabel {
	font-weight: bold;
	color: #000000;
}
QWidget#Line {
	border: none;
	background: #ABADAE;
	border-radius: 0px;
	min-height: 1px;
	max-height: 1px;
}
)";
#pragma endregion

};

