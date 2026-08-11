#pragma once

#include "TitleBar.h"
#include "TopBar.h"
#include "SideBarPanel.h"
#include "database/ProcessRepository.h"
#include "database/DatabaseManager.h"

#include <QMainWindow>
#include <QString>
#include <QSplitter>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow() override = default;

private:
	TitleBar* m_titleBar = nullptr;
	TopBar* m_topBar = nullptr;
	QSplitter* m_bodySplitter;
	SideBarPanel* m_sideBarPanel;
	ProcessRepository m_processRepo = ProcessRepository(DatabaseManager::instance());
	QWidget* m_3dPage;
	const QString mainWindowQSS = R"(
QWidget#MainWindow {
	background: #FFFFFF;
}
)";
	void setWindowUI(QWidget* parent);
};

