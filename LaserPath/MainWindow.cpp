#include "MainWindow.h"
#include "TitleBar.h"
#include "TopBar.h"
#include "SideBarPanel.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowUI(parent);
}

void MainWindow::setWindowUI(QWidget* parent)
{
	resize(1920, 1280);
	setMinimumSize(1920, 1280);
	setWindowFlags(Qt::FramelessWindowHint);

	QWidget* mainWindow = new QWidget(parent);
	mainWindow->setObjectName("MainWindow");
	mainWindow->setStyleSheet(mainWindowQSS);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainWindow);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	m_titleBar = new TitleBar(mainWindow);
	m_topBar = new TopBar(mainWindow);


	connect(m_titleBar, &TitleBar::minClicked, this, [this]() {
		if (!isMinimized())
			showMinimized();
		});
	connect(m_titleBar, &TitleBar::maxClicked, this, [this]() {
		if (!isMaximized())
			showMaximized();
		else
			showNormal();
		m_titleBar->setMaxIcon(isMaximized());
		});
	connect(m_titleBar, &TitleBar::closeClicked, this, [this]() {
		close();
		});

	mainLayout->addWidget(m_titleBar);
	mainLayout->addWidget(m_topBar);

	setCentralWidget(mainWindow);

	m_bodySplitter = new QSplitter(Qt::Horizontal, mainWindow);
	m_bodySplitter->setHandleWidth(1);

	m_sideBarPanel = new SideBarPanel(&m_processRepo, &m_machineRepo);
	m_3dPage = new QWidget();

	m_bodySplitter->addWidget(m_sideBarPanel);
	m_bodySplitter->addWidget(m_3dPage);

	m_sideBarPanel->setFixedWidth(375);
	m_bodySplitter->setCollapsible(0, false);

	mainLayout->addWidget(m_bodySplitter);
}

