#pragma once

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QMouseEvent>

class SectionHeader : public QWidget
{
	Q_OBJECT

public:
	explicit SectionHeader(const QString& iconPath,
		const QString& title,
		const QString& actionIconPath = {},
		QWidget* parent = nullptr);
	~SectionHeader() override = default;

private:

	QPushButton* m_actionBtn{ nullptr };

	bool m_pressed{ false };

#pragma region MyRegion
	const QString sectionHeaderQSS = R"(
QWidget#SectionHeader {
	background: #F5F5F5;
	border: 1px solid #ABADAE;
	min-height: 30px;
	max-height: 30px;
}
)";
#pragma endregion

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

signals:
	void clicked();
};

