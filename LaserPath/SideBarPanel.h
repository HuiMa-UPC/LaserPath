#pragma once

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include <QStackedWidget>
#include <QScrollArea>

#include "SectionHeader.h"
#include "ParamRow.h"
#include "ParamHeader.h"

class ProcessRepository;
class QMenu;
struct ProcessProfile;

class SideBarPanel : public QWidget
{
	Q_OBJECT

public:
	SideBarPanel(ProcessRepository* repo, QWidget *parent = nullptr);
	~SideBarPanel() override = default;

private:

#pragma region Device
	SectionHeader* m_deviceHeader = nullptr;
	QWidget* m_deviceContent = nullptr;

	QLabel* m_deviceImage = nullptr;
	QPushButton* m_editBtn = nullptr;
	QPushButton* m_selectBtn = nullptr;

	ParamRow* m_diameterRow = nullptr;
	ParamRow* m_stickOutRow = nullptr;
#pragma endregion

#pragma region Process
	SectionHeader* m_processHeader = nullptr;
	QWidget* m_processContent = nullptr;

	QPushButton* m_allResetBtn = nullptr;
	QPushButton* m_processSelectBtn = nullptr;
	QPushButton* m_processSaveBtn = nullptr;
	QPushButton* m_processAddBtn = nullptr;
	QPushButton* m_processRenameBtn = nullptr;
	QPushButton* m_processDeleteBtn = nullptr;
	ProcessRepository* m_processRepo = nullptr;
	QMenu* m_processSelectMenu = nullptr;
	int m_currentProfileId = 0;

	QTabBar* m_tapBar = nullptr;

	ParamHeader* m_sliceTitle{ nullptr };

	ParamRow* m_layerHeight{ nullptr };

	ParamRow* m_firstLayerHeight{ nullptr };

	ParamRow* m_autoRange{ nullptr };

	ParamRow* m_zMin{ nullptr };

	ParamRow* m_zMax{ nullptr };

	ParamHeader* m_precisionTitle{ nullptr };

	ParamRow* m_minContourLen{ nullptr };

	ParamRow* m_collinearPrecision{ nullptr };

	ParamRow* m_gapCloseRadius{ nullptr };

	ParamHeader* m_offsetTitle{ nullptr };

	ParamRow* m_enableOffset{ nullptr };

	ParamRow* m_outerWallWidth{ nullptr };

	ParamRow* m_innerWallWidth{ nullptr };

	ParamRow* m_baseContour{ nullptr };

	ParamRow* m_autoOffset{ nullptr };

	ParamRow* m_offsetCount{ nullptr };

	ParamRow* m_offsetContourComp{ nullptr };

	ParamRow* m_startAngle{ nullptr };

	ParamRow* m_startPointStrategy{ nullptr };

	ParamRow* m_angleIncrement{ nullptr };

	ParamRow* m_lengthIncrement{ nullptr };

	ParamHeader* m_fillTitle{ nullptr };

	ParamRow* m_beadWidth{ nullptr };

	ParamRow* m_fillType{ nullptr };

	ParamRow* m_adaptiveOverlap{ nullptr };

	ParamRow* m_maxOverlapRate{ nullptr };

	ParamRow* m_minOverlapRate{ nullptr };

	ParamRow* m_overlapRate{ nullptr };

	ParamRow* m_marginStrategy{ nullptr };

	ParamRow* m_scanDirection{ nullptr };

	ParamRow* m_scanAngle{ nullptr };

	ParamRow* m_interlayerRotation{ nullptr };

	ParamRow* m_fillAreaComp{ nullptr };

	ParamRow* m_endExtension{ nullptr };

#pragma endregion

#pragma region QSS
	const QString deviceCardQSS = R"(
QWidget#DeviceCard {
	background: transparent;
	border: 1px solid #ABADAE;
	border-radius: 5px;
}
QWidget#DeviceCard:hover {
	border: 1px solid #00AE42;
}
QPushButton#SelectBtn {
	border: none;
	border-radius: 0px;
	background: transparent;
	height: 25px;
}
QPushButton#SelectBtn:hover {
	border: none;
	border-radius: 0px;
	background: #EDFAF2;
}
)";
	const QString processTopBar = R"(
QWidget#ProcessTopBar {
	background: transparent;
}
QPushButton#SelectBtn {
	background: transparent;
	border: 1px solid #ABADAE;
	border-radius: 0px;
	min-height: 25px;
	max-height: 25px;
    text-align: left;
	padding-left: 6px;
	padding-right: 6px;
}
QPushButton#SelectBtn:hover {
	border: 1px solid #00AE42;
}
		QMenu {
			background: #FFFFFF;
			border: 1px solid #D0D0D0;
			padding: 0px 2px;
		}
		QMenu::item {
			padding: 6px 24px 6px 6px;
			min-width: 265px;
			max-width: 265px;
		}
		QMenu::item:selected {
			border: 1px solid #00AE42;
		}
		QMenu::item:checked {
			background: #EDFAF2;
		}
		QMenu::indicator {
			width: 14px;
			height: 14px;
			margin-left: 6px;
		}
		QMenu::indicator:checked {
			image: url(:/icons/checkmark_green);
		}
)";
	const QString processTapBar = R"(
        QTabBar {
            background: transparent;
        }
        QTabBar::tab {
            background: transparent;
            color: #6B6B6B;
            border: none;

            border-bottom: 3px solid transparent;
            padding: 3px 2px;
            margin-right: 20px;
        }
        QTabBar::tab:hover {
            background: transparent;
            color: #6B6B6B;
        }
        QTabBar::tab:selected {
            color: #000000;
            font-weight: bold;
            border-bottom: 3px solid #00AE42;
        }
    )";
	const QString sideBarPanelQSS = R"(
QWidget#SideBarPanel {
    background: #FFFFFF;
}
)";
	const QString panelScrollQSS = R"(

QScrollArea#PanelScroll {
    background: #FFFFFF;
    border: none;
}
QScrollArea#PanelScroll QStackedWidget {
    background: #FFFFFF;
}
QScrollArea#PanelScroll QScrollBar:vertical {
    width: 6px;
    background: transparent;
    margin: 0;
}
QScrollArea#PanelScroll QScrollBar::handle:vertical {
    background: #D0D0D0;
    border-radius: 3px;
    min-height: 32px;
}
QScrollArea#PanelScroll QScrollBar::handle:vertical:hover {
    background: #A0A0A0;
}
QScrollArea#PanelScroll QScrollBar::add-line:vertical,
QScrollArea#PanelScroll QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollArea#PanelScroll QScrollBar::add-page:vertical,
QScrollArea#PanelScroll QScrollBar::sub-page:vertical {
    background: transparent;
}
)";
#pragma endregion

	QWidget* createDeviceCard(QWidget* parent = nullptr);

	QWidget* createDeviceContent(QWidget* parent = nullptr);

	QWidget* createProcessTopBar(QWidget* parent = nullptr);

	QTabBar* createProcessTapBar(QWidget* parent = nullptr);

	QScrollArea* createProcessStack(QWidget* parent = nullptr);

	QWidget* createStrategyPage(QWidget* parent = nullptr);

	QWidget* createSpeedPage(QWidget* parent = nullptr);

	QWidget* createProcessCard(QWidget* parent = nullptr);

	QWidget* createProcessContent(QWidget* parent = nullptr);

	void refreshProcessList();
	void loadFromProfile(const ProcessProfile& p);
	void setupModifiedCallbacks();
	void updateAllResetState();

private slots:
	void onNewProcess();
	void onDeleteProcess();
	void onRenameProcess();
		void onSaveProcess();
};
