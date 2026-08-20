#pragma once

#include <QDialog>
#include <QVector>
#include <QPushButton>
#include <QMenu>
#include <QTabBar>
#include <QLabel>
#include <QStackedWidget>
#include <QPlainTextEdit>

#include "ParamRow.h"
#include "database/MachineProfile.h"

class MachineRepository;
struct MachineProfile;

class EditMachineDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditMachineDialog(MachineRepository* repo, QWidget* parent = nullptr);

private slots:
    void onMachineSave();
    void onMachineAdd();
    void onMachineDelete();
    void onMachineReset();

private:

#pragma region QSS
    const QString kMachineTopBar = R"(
QWidget#MachineTopBar {
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
    const QString kMachineTabBar = R"(
        QTabBar {
            background: transparent;
        }
        QTabBar::tab {
            background: transparent;
            color: #6B6B6B;
            border: none;
            min-width: 60px;

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
#pragma endregion

    QWidget* createTopBar(QWidget* parent = nullptr);
    QTabBar* createTabBar(QWidget* parent = nullptr);
    QWidget* createPageBasicInfo(QWidget* parent = nullptr);
    QWidget* createImageCard(QWidget* parent = nullptr);
    QWidget* createPageGCode(QWidget* parent = nullptr);
    QWidget* createPageCapability(QWidget* parent = nullptr);
    QWidget* createPageNotes(QWidget* parent = nullptr);

    void loadFromProfile(const MachineProfile& p);
    void collectToProfile(MachineProfile& p) const;
    void refreshMachineList();
    void setupDirtyTracking();
    void updateAllResetState();

    QPushButton* m_allResetBtn = nullptr;
    QPushButton* m_machineSelectBtn = nullptr;
    QPushButton* m_machineSaveBtn = nullptr;
    QPushButton* m_machineAddBtn = nullptr;
    QPushButton* m_machineDeleteBtn = nullptr;

    QMenu* m_machineMenu = nullptr;
    QTabBar* m_tabBar = nullptr;
    QStackedWidget* m_pageStack = nullptr;

    ParamRow* m_name = nullptr;
    ParamRow* m_kinematics = nullptr;
    ParamRow* m_heatSource = nullptr;
    ParamRow* m_imageRow = nullptr;
    ParamRow* m_maxHeight = nullptr;
    ParamRow* m_maxX = nullptr;
    ParamRow* m_maxY = nullptr;
    ParamRow* m_originX = nullptr;
    ParamRow* m_originY = nullptr;

    ParamRow* m_maxSpeedX = nullptr;
    ParamRow* m_maxSpeedY = nullptr;
    ParamRow* m_maxSpeedZ = nullptr;
    ParamRow* m_maxSpeedE = nullptr;

    QPushButton* m_loadImageBtn = nullptr;
    QLabel* m_image = nullptr;

    // G-code editors (promoted from locals)
    QPlainTextEdit* m_printStartGcodeEdit = nullptr;
    QPlainTextEdit* m_printEndGcodeEdit = nullptr;
    QPlainTextEdit* m_pathStartGcodeEdit = nullptr;
    QPlainTextEdit* m_pathEndGcodeEdit = nullptr;
    QPlainTextEdit* m_layerStartGcodeEdit = nullptr;
    QPlainTextEdit* m_layerEndGcodeEdit = nullptr;
    QPlainTextEdit* m_interlayerCoolingGcodeEdit = nullptr;

    // G-code per-field reset buttons
    QPushButton* m_printStartGcodeResetBtn = nullptr;
    QPushButton* m_printEndGcodeResetBtn = nullptr;
    QPushButton* m_pathStartGcodeResetBtn = nullptr;
    QPushButton* m_pathEndGcodeResetBtn = nullptr;
    QPushButton* m_layerStartGcodeResetBtn = nullptr;
    QPushButton* m_layerEndGcodeResetBtn = nullptr;
    QPushButton* m_interlayerCoolingGcodeResetBtn = nullptr;

    // Notes editor (promoted from local)
    QPlainTextEdit* m_notesEdit = nullptr;
    QPushButton* m_notesResetBtn = nullptr;

    // OK/Cancel buttons
    QPushButton* m_okBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;

    // Repository & state
    MachineRepository* m_repo = nullptr;
    int m_currentProfileId = 0;
    QString m_currentImagePath;
    QByteArray m_currentImageData;
    MachineProfile m_savedProfile;

    QVector<QString> m_kinematicsItems = { "三轴 CNC", "五轴 CNC", "六轴 KUKA", "八轴 KUKA" };
    QVector<QString> m_heatSourceItems = { "激光", "电弧" };


};
