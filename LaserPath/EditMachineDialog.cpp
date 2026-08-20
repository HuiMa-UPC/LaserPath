#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QFile>
#include <QDateTime>
#include <QFileDialog>
#include <QtCore/QBuffer>

#include "Styles.h"
#include "ParamHeader.h"
#include "ParamRow.h"
#include "EditMachineDialog.h"
#include "PlateGridPreview.h"
#include "CustomDialog.h"
#include "database/MachineRepository.h"

const QString kEditMachineDialog = R"(
QDialog#EditMachineDialog {
    background: #FFFFFF;
    max-width: 410px;
    min-width: 410px;
    max-height: 550px;
    min-height: 550px;
}
QWidget#ImageCard {
	border: 1px solid #ABADAE;
	min-height: 120px;
	max-height: 120px;
	min-width: 120px;
	max-width: 120px;
	border-radius: 5px;
}
QWidget#Page {
    background: #FFFFFF;
}
QPushButton#LoadImageBtn {
	border: none;
	border-radius: 0px;
	background: transparent;
	height: 25px;
}
QPushButton#LoadImageBtn:hover {
	border: none;
	border-radius: 0px;
	background: #EDFAF2;
}
)";

const QString kPanelScroll = R"(

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
QPlainTextEdit {
    border: 1px solid #D9D9D9;
    border-radius: 0px;
    color: #262E30;
    margin: 2px 10px;
}
QPlainTextEdit:focus {
    border: 1px solid #00AE42;
}
)";

EditMachineDialog::EditMachineDialog(MachineRepository* repo, QWidget* parent)
    : QDialog(parent)
    , m_repo(repo)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("EditMachineDialog");
    setStyleSheet(kEditMachineDialog);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);

    layout->addWidget(createTopBar(this));

    m_tabBar = createTabBar(this);
    layout->addWidget(m_tabBar);

    m_pageStack = new QStackedWidget(this);
    m_pageStack->addWidget(createPageBasicInfo(m_pageStack));   // index 0
    m_pageStack->addWidget(createPageGCode(m_pageStack));       // index 1
    m_pageStack->addWidget(createPageCapability(m_pageStack));  // index 2
    m_pageStack->addWidget(createPageNotes(m_pageStack));       // index 3
    m_pageStack->setCurrentIndex(0);
    layout->addWidget(m_pageStack, 1);

    connect(m_tabBar, &QTabBar::currentChanged,
        m_pageStack, &QStackedWidget::setCurrentIndex);

    // ── OK / Cancel buttons ──
    auto* buttonRow = new QWidget(this);
    auto* btnLayout = new QHBoxLayout(buttonRow);
    btnLayout->setContentsMargins(10, 5, 10, 10);
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton(QStringLiteral("取消"), buttonRow);
    m_okBtn = new QPushButton(QStringLiteral("确定"), buttonRow);
    m_okBtn->setDefault(true);

    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_okBtn);

    layout->addWidget(buttonRow);

    // ── Connections ──
    connect(m_machineSaveBtn, &QPushButton::clicked, this, &EditMachineDialog::onMachineSave);
    connect(m_machineAddBtn, &QPushButton::clicked, this, &EditMachineDialog::onMachineAdd);
    connect(m_machineDeleteBtn, &QPushButton::clicked, this, &EditMachineDialog::onMachineDelete);
    connect(m_allResetBtn, &QPushButton::clicked, this, &EditMachineDialog::onMachineReset);

    connect(m_machineSelectBtn, &QPushButton::clicked, this, [this]() {
        refreshMachineList();
        QPoint pos = m_machineSelectBtn->mapToGlobal(QPoint(0, m_machineSelectBtn->height()));
        m_machineMenu->exec(pos);
    });

    connect(m_okBtn, &QPushButton::clicked, this, [this]() { accept(); });
    connect(m_cancelBtn, &QPushButton::clicked, this, [this]() { reject(); });

    // ── 加载图像 ──
    connect(m_loadImageBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this,
            QStringLiteral("选择设备图像"), QString(),
            QStringLiteral("图像文件 (*.png *.jpg *.jpeg *.bmp)"));
        if (path.isEmpty()) return;

        QPixmap pm(path);
        if (pm.isNull()) return;

        m_image->setPixmap(pm.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_currentImagePath = path;

        // 统一编码为 PNG 存入数据库
        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QIODevice::WriteOnly);
        pm.save(&buf, "PNG");
        m_currentImageData = ba;

        updateAllResetState();
    });

    // ── G-code 逐字段 reset ──
    connect(m_printStartGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_printStartGcodeEdit->setPlainText(m_savedProfile.printStartGcode);
    });
    connect(m_printEndGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_printEndGcodeEdit->setPlainText(m_savedProfile.printEndGcode);
    });
    connect(m_pathStartGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_pathStartGcodeEdit->setPlainText(m_savedProfile.pathStartGcode);
    });
    connect(m_pathEndGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_pathEndGcodeEdit->setPlainText(m_savedProfile.pathEndGcode);
    });
    connect(m_layerStartGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_layerStartGcodeEdit->setPlainText(m_savedProfile.layerStartGcode);
    });
    connect(m_layerEndGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_layerEndGcodeEdit->setPlainText(m_savedProfile.layerEndGcode);
    });
    connect(m_interlayerCoolingGcodeResetBtn, &QPushButton::clicked, this, [this]() {
        m_interlayerCoolingGcodeEdit->setPlainText(m_savedProfile.interlayerCoolingGcode);
    });

    // ── 备注 reset ──
    connect(m_notesResetBtn, &QPushButton::clicked, this, [this]() {
        m_notesEdit->setPlainText(m_savedProfile.notes);
    });

    setupDirtyTracking();

    // ── 启动时自动加载第一条设备数据 ──
    if (m_repo) {
        refreshMachineList();
        auto names = m_repo->listAllName();
        if (!names.isEmpty()) {
            m_currentProfileId = names.first().first;
            MachineProfile profile = m_repo->getById(m_currentProfileId);
            loadFromProfile(profile);
            m_machineSelectBtn->setText(profile.name);
        }
    }
}

QWidget* EditMachineDialog::createTopBar(QWidget* parent)
{
    QWidget* content = new QWidget(parent);
    content->setObjectName("MachineTopBar");
    content->setStyleSheet(kMachineTopBar + Styles::kIconBtn);

    QHBoxLayout* layout = new QHBoxLayout(content);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_allResetBtn = new QPushButton(content);
    m_allResetBtn->setIcon(QIcon(":/icons/reset"));
    m_allResetBtn->setToolTip(QStringLiteral("点击以将所有设置还原到最后一次保存的版本。"));
    m_allResetBtn->setObjectName("IconBtn");
    m_allResetBtn->setEnabled(false);

    m_machineSelectBtn = new QPushButton(content);
    m_machineSelectBtn->setObjectName("SelectBtn");
    m_machineSelectBtn->setIcon(QIcon(":/icons/arrow"));
    m_machineSelectBtn->setText("Rongsu Laser One");
    m_machineSelectBtn->setToolTip(QStringLiteral("切换选择其他设备配置"));
    m_machineSelectBtn->setFocusPolicy(Qt::NoFocus);

    m_machineSaveBtn = new QPushButton(content);
    m_machineSaveBtn->setIcon(QIcon(":/icons/sideBar_save"));
    m_machineSaveBtn->setToolTip(QStringLiteral("保存设备配置"));
    m_machineSaveBtn->setObjectName("IconBtn");

    m_machineAddBtn = new QPushButton(content);
    m_machineAddBtn->setIcon(QIcon(":/icons/add"));
    m_machineAddBtn->setToolTip(QStringLiteral("新建设备配置"));
    m_machineAddBtn->setObjectName("IconBtn");

    m_machineDeleteBtn = new QPushButton(content);
    m_machineDeleteBtn->setIcon(QIcon(":/icons/delete"));
    m_machineDeleteBtn->setToolTip(QStringLiteral("删除设备配置"));
    m_machineDeleteBtn->setObjectName("IconBtn");

    layout->addWidget(m_allResetBtn);
    layout->addWidget(m_machineSelectBtn, 1);
    layout->addWidget(m_machineSaveBtn);
    layout->addWidget(m_machineAddBtn);
    layout->addWidget(m_machineDeleteBtn);

    return content;
}

QTabBar* EditMachineDialog::createTabBar(QWidget* parent) {
    QTabBar* tabBar = new QTabBar(parent);

    tabBar->addTab(QStringLiteral("基础信息"));
    tabBar->addTab(QStringLiteral("G-code"));
    tabBar->addTab(QStringLiteral("运动能力"));
    tabBar->addTab(QStringLiteral("机器备注"));

    tabBar->setExpanding(false);
    tabBar->setStyleSheet(kMachineTabBar);
    return tabBar;
}

QWidget* EditMachineDialog::createPageBasicInfo(QWidget* parent)
{
    auto* scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("PanelScroll");
    scroll->setStyleSheet(kPanelScroll + Styles::kIconBtn);

    auto* page = new QWidget(scroll);
    page->setObjectName("Page");
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(5);

    ParamHeader* deviceInfo = new ParamHeader(":/icons/info", "设备信息", page);
    m_name = new ParamRow("名称", QString(""), page);
    m_imageRow = new ParamRow("外观", createImageCard(page), page);
    m_kinematics = new ParamRow("运动构型", m_kinematicsItems, page);
    m_heatSource = new ParamRow("热源类型", m_heatSourceItems, page);

    ParamHeader* printingSpace = new ParamHeader(":/icons/info", "可打印空间", page);
    m_maxHeight = new ParamRow("可打印高度", "mm", 0.0, 99999.0, 0.1, 1, page);
    m_maxX = new ParamRow("基板 X 方向长度", "mm", 0.0, 99999.0, 0.1, 1, page);
    m_maxY = new ParamRow("基板 Y 方向长度", "mm", 0.0, 99999.0, 0.1, 1, page);
    m_originX = new ParamRow("原点 X", "mm", 0.0, 9999.0, 0.1, 1, page);
    m_originY = new ParamRow("原点 Y", "mm", 0.0, 9999.0, 0.1, 1, page);

    layout->addWidget(deviceInfo);
    layout->addWidget(m_name);
    layout->addWidget(m_imageRow);
    layout->addWidget(m_kinematics);
    layout->addWidget(m_heatSource);
    layout->addSpacing(10);
    layout->addWidget(printingSpace);
    layout->addWidget(m_maxHeight);
    layout->addWidget(m_maxX);
    layout->addWidget(m_maxY);
    layout->addWidget(m_originX);
    layout->addWidget(m_originY);

    // ── Plate preview below form ──
    auto* plateGridPreview = new PlateGridPreview(page);
    plateGridPreview->setMinimumSize(360, 260);
    plateGridPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(plateGridPreview, 1);

    // ── Connect spinBox changes → update preview ──
    auto updatePreview = [this, plateGridPreview]() {
        double sx = qobject_cast<QDoubleSpinBox*>(m_maxX->spinBox())->value();
        double sy = qobject_cast<QDoubleSpinBox*>(m_maxY->spinBox())->value();
        double ox = qobject_cast<QDoubleSpinBox*>(m_originX->spinBox())->value();
        double oy = qobject_cast<QDoubleSpinBox*>(m_originY->spinBox())->value();
        plateGridPreview->setBedParams(sx, sy, ox, oy);
    };

    auto* sxSpin = qobject_cast<QDoubleSpinBox*>(m_maxX->spinBox());
    auto* sySpin = qobject_cast<QDoubleSpinBox*>(m_maxY->spinBox());
    auto* oxSpin = qobject_cast<QDoubleSpinBox*>(m_originX->spinBox());
    auto* oySpin = qobject_cast<QDoubleSpinBox*>(m_originY->spinBox());

    connect(sxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, updatePreview);
    connect(sySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, updatePreview);
    connect(oxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, updatePreview);
    connect(oySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, updatePreview);

    updatePreview();

    scroll->setWidget(page);
    return scroll;
}

QWidget* EditMachineDialog::createImageCard(QWidget* parent)
{
    QWidget* content = new QWidget(parent);
    content->setObjectName("ImageCard");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(5, 5, 5, 5);
    contentLayout->setSpacing(0);
    contentLayout->setAlignment(Qt::AlignCenter);

    QWidget* imageBorder = new QWidget(content);

    QHBoxLayout* imageLayout = new QHBoxLayout(imageBorder);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(0);
    imageLayout->setAlignment(Qt::AlignCenter);
    m_image = new QLabel(imageBorder);
    m_image->setPixmap(QIcon(":/icons/image").pixmap(100, 100));
    imageLayout->addWidget(m_image);

    m_loadImageBtn = new QPushButton(content);
    m_loadImageBtn->setText(QStringLiteral("加载图像..."));
    m_loadImageBtn->setObjectName("LoadImageBtn");

    contentLayout->addWidget(imageBorder);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_loadImageBtn);

    return content;
}

QWidget* EditMachineDialog::createPageGCode(QWidget* parent)
{
    auto* scroll = new QScrollArea(parent);

    scroll->setWidgetResizable(true);
    scroll->setObjectName("PanelScroll");
    scroll->setStyleSheet(kPanelScroll + Styles::kIconBtn);

    auto* page = new QWidget(scroll);
    page->setObjectName("Page");

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 构建一个区块：标题 + reset 按钮行 + 文本编辑器
    auto addGcodeSection = [&](const QString& title,
            QPushButton*& resetBtn, QPlainTextEdit*& edit) {
        ParamHeader* header = new ParamHeader(":/icons/cogPage", title, page);

        resetBtn = new QPushButton(page);
        resetBtn->setIcon(QIcon(":/icons/reset"));
        resetBtn->setObjectName("IconBtn");

        auto* hLayout = new QHBoxLayout();
        hLayout->addSpacing(10);
        hLayout->addWidget(resetBtn);
        hLayout->addStretch(1);

        edit = new QPlainTextEdit(page);
        edit->setFixedHeight(120);
        edit->setTabStopDistance(20);

        layout->addWidget(header);
        layout->addLayout(hLayout);
        layout->addWidget(edit);
    };

    // ========== 打印起始 G-code ==========
    addGcodeSection(QStringLiteral("打印起始 G-code"),
        m_printStartGcodeResetBtn, m_printStartGcodeEdit);

    // ========== 打印结束 G-code ==========
    addGcodeSection(QStringLiteral("打印结束 G-code"),
        m_printEndGcodeResetBtn, m_printEndGcodeEdit);

    // ========== 轨迹起始 G-code ==========
    addGcodeSection(QStringLiteral("轨迹起始 G-code"),
        m_pathStartGcodeResetBtn, m_pathStartGcodeEdit);

    // ========== 轨迹结束 G-code ==========
    addGcodeSection(QStringLiteral("轨迹结束 G-code"),
        m_pathEndGcodeResetBtn, m_pathEndGcodeEdit);

    // ========== 层开始 G-code ==========
    addGcodeSection(QStringLiteral("层开始 G-code"),
        m_layerStartGcodeResetBtn, m_layerStartGcodeEdit);

    // ========== 层结束 G-code ==========
    addGcodeSection(QStringLiteral("层结束 G-code"),
        m_layerEndGcodeResetBtn, m_layerEndGcodeEdit);

    // ========== 特殊层间冷却 G-code ==========
    addGcodeSection(QStringLiteral("特殊层间冷却 G-code"),
        m_interlayerCoolingGcodeResetBtn, m_interlayerCoolingGcodeEdit);

    layout->addSpacing(20);

    scroll->setWidget(page);
    return scroll;
}

QWidget* EditMachineDialog::createPageCapability(QWidget* parent)
{
    auto* scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("PanelScroll");
    scroll->setStyleSheet(kPanelScroll + Styles::kIconBtn);

    auto* page = new QWidget(scroll);
    page->setObjectName("Page");
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    ParamHeader* speedHeader = new ParamHeader(":/icons/speed", "速度限制", page);
    m_maxSpeedX = new ParamRow("X 轴最大速度", "mm/s", 0.0, 99999.0, 1.0, 1, page);
    m_maxSpeedY = new ParamRow("Y 轴最大速度", "mm/s", 0.0, 99999.0, 1.0, 1, page);
    m_maxSpeedZ = new ParamRow("Z 轴最大速度", "mm/s", 0.0, 99999.0, 1.0, 1, page);
    m_maxSpeedE = new ParamRow("E 轴最大速度", "mm/s", 0.0, 99999.0, 1.0, 1, page);

    layout->addWidget(speedHeader);
    layout->addWidget(m_maxSpeedX);
    layout->addWidget(m_maxSpeedY);
    layout->addWidget(m_maxSpeedZ);
    layout->addWidget(m_maxSpeedE);

    layout->addStretch();

    scroll->setWidget(page);
    return scroll;
}

QWidget* EditMachineDialog::createPageNotes(QWidget* parent)
{
    auto* scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("PanelScroll");
    scroll->setStyleSheet(kPanelScroll + Styles::kIconBtn);

    auto* page = new QWidget(scroll);
    page->setObjectName("Page");
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    ParamHeader* header = new ParamHeader(":/icons/notes", "备注", page);

    m_notesResetBtn = new QPushButton(page);
    m_notesResetBtn->setIcon(QIcon(":/icons/reset"));
    m_notesResetBtn->setObjectName("IconBtn");

    auto* hNotesLayout = new QHBoxLayout();
    hNotesLayout->addSpacing(10);
    hNotesLayout->addWidget(m_notesResetBtn);
    hNotesLayout->addStretch(1);

    m_notesEdit = new QPlainTextEdit(page);
    m_notesEdit->setFixedHeight(160);
    m_notesEdit->setTabStopDistance(20);

    layout->addWidget(header);
    layout->addLayout(hNotesLayout);
    layout->addWidget(m_notesEdit);

    layout->addStretch();

    scroll->setWidget(page);
    return scroll;
}

/************************** 数据库 → UI **************************/
void EditMachineDialog::loadFromProfile(const MachineProfile& p)
{
    // 先更新基准，避免 widget 更新回调中与旧值比较
    m_savedProfile = p;

    m_name->setDbValue(p.name);
    m_kinematics->setDbValue(p.kinematics);
    m_heatSource->setDbValue(p.heatSource);
    m_maxHeight->setDbValue(p.maxHeight);
    m_maxX->setDbValue(p.maxX);
    m_maxY->setDbValue(p.maxY);
    m_originX->setDbValue(p.originX);
    m_originY->setDbValue(p.originY);

    // 图像
    m_currentImagePath = p.imagePath;
    m_currentImageData = p.imageData;
    if (!p.imageData.isEmpty()) {
        QPixmap pm;
        pm.loadFromData(p.imageData);
        m_image->setPixmap(pm.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_image->setPixmap(QIcon(":/icons/image").pixmap(100, 100));
    }

    // G-code
    m_printStartGcodeEdit->setPlainText(p.printStartGcode);
    m_printEndGcodeEdit->setPlainText(p.printEndGcode);
    m_pathStartGcodeEdit->setPlainText(p.pathStartGcode);
    m_pathEndGcodeEdit->setPlainText(p.pathEndGcode);
    m_layerStartGcodeEdit->setPlainText(p.layerStartGcode);
    m_layerEndGcodeEdit->setPlainText(p.layerEndGcode);
    m_interlayerCoolingGcodeEdit->setPlainText(p.interlayerCoolingGcode);

    // 运动能力
    m_maxSpeedX->setDbValue(p.maxSpeedX);
    m_maxSpeedY->setDbValue(p.maxSpeedY);
    m_maxSpeedZ->setDbValue(p.maxSpeedZ);
    m_maxSpeedE->setDbValue(p.maxSpeedE);

    // 备注
    m_notesEdit->setPlainText(p.notes);
}

/************************** UI → 数据库 **************************/
void EditMachineDialog::collectToProfile(MachineProfile& p) const
{
    p.name = m_name->lineEdit()->text();
    p.kinematics = m_kinematics->currentIndex();
    p.heatSource = m_heatSource->currentIndex();
    p.maxHeight = qobject_cast<QDoubleSpinBox*>(m_maxHeight->spinBox())->value();
    p.maxX = qobject_cast<QDoubleSpinBox*>(m_maxX->spinBox())->value();
    p.maxY = qobject_cast<QDoubleSpinBox*>(m_maxY->spinBox())->value();
    p.originX = qobject_cast<QDoubleSpinBox*>(m_originX->spinBox())->value();
    p.originY = qobject_cast<QDoubleSpinBox*>(m_originY->spinBox())->value();
    p.imagePath = m_currentImagePath;
    p.imageData = m_currentImageData;
    p.printStartGcode = m_printStartGcodeEdit->toPlainText();
    p.printEndGcode = m_printEndGcodeEdit->toPlainText();
    p.pathStartGcode = m_pathStartGcodeEdit->toPlainText();
    p.pathEndGcode = m_pathEndGcodeEdit->toPlainText();
    p.layerStartGcode = m_layerStartGcodeEdit->toPlainText();
    p.layerEndGcode = m_layerEndGcodeEdit->toPlainText();
    p.interlayerCoolingGcode = m_interlayerCoolingGcodeEdit->toPlainText();
    p.maxSpeedX = qobject_cast<QDoubleSpinBox*>(m_maxSpeedX->spinBox())->value();
    p.maxSpeedY = qobject_cast<QDoubleSpinBox*>(m_maxSpeedY->spinBox())->value();
    p.maxSpeedZ = qobject_cast<QDoubleSpinBox*>(m_maxSpeedZ->spinBox())->value();
    p.maxSpeedE = qobject_cast<QDoubleSpinBox*>(m_maxSpeedE->spinBox())->value();
    p.notes = m_notesEdit->toPlainText();
}

/************************** 刷新设备选择菜单 **************************/
void EditMachineDialog::refreshMachineList()
{
    if (!m_repo) return;

    auto names = m_repo->listAllName();

    delete m_machineMenu;
    m_machineMenu = new QMenu(m_machineSelectBtn);

    if (names.isEmpty()) {
        m_machineSelectBtn->setText(QStringLiteral("(无设备)"));
        m_machineMenu->addAction(QStringLiteral("(无设备，请新建)"))->setEnabled(false);
        m_currentProfileId = 0;
        return;
    }

    for (const auto& pair : names) {
        QAction* act = m_machineMenu->addAction(pair.second);
        act->setCheckable(true);
        if (pair.first == m_currentProfileId)
            act->setChecked(true);
        connect(act, &QAction::triggered, this, [this, id = pair.first, name = pair.second]() {
            m_currentProfileId = id;
            m_machineSelectBtn->setText(name);
            const auto actions = m_machineMenu->actions();
            for (int j = 0; j < actions.size(); ++j)
                actions.at(j)->setChecked(j == actions.indexOf(qobject_cast<QAction*>(sender())));
            MachineProfile profile = m_repo->getById(id);
            loadFromProfile(profile);
        });
    }

    if (m_currentProfileId > 0) {
        m_machineSelectBtn->setText(m_repo->getById(m_currentProfileId).name);
    } else {
        m_machineSelectBtn->setText(QStringLiteral("选择设备..."));
    }
}

/************************** 脏状态跟踪 **************************/
void EditMachineDialog::setupDirtyTracking()
{
    auto cb = [this]() { updateAllResetState(); };

    m_name->setModifiedCallback(cb);
    m_kinematics->setModifiedCallback(cb);
    m_heatSource->setModifiedCallback(cb);
    m_maxHeight->setModifiedCallback(cb);
    m_maxX->setModifiedCallback(cb);
    m_maxY->setModifiedCallback(cb);
    m_originX->setModifiedCallback(cb);
    m_originY->setModifiedCallback(cb);
    m_maxSpeedX->setModifiedCallback(cb);
    m_maxSpeedY->setModifiedCallback(cb);
    m_maxSpeedZ->setModifiedCallback(cb);
    m_maxSpeedE->setModifiedCallback(cb);

    // QPlainTextEdit fields — compare against saved profile
    connect(m_printStartGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_printEndGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_pathStartGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_pathEndGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_layerStartGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_layerEndGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_interlayerCoolingGcodeEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
    connect(m_notesEdit, &QPlainTextEdit::textChanged, this, [this]() { updateAllResetState(); });
}

void EditMachineDialog::updateAllResetState()
{
    bool any = m_name->isModified()
        || m_kinematics->isModified()
        || m_heatSource->isModified()
        || m_maxHeight->isModified()
        || m_maxX->isModified()
        || m_maxY->isModified()
        || m_originX->isModified()
        || m_originY->isModified()
        || m_maxSpeedX->isModified()
        || m_maxSpeedY->isModified()
        || m_maxSpeedZ->isModified()
        || m_maxSpeedE->isModified()
        || m_printStartGcodeEdit->toPlainText() != m_savedProfile.printStartGcode
        || m_printEndGcodeEdit->toPlainText() != m_savedProfile.printEndGcode
        || m_pathStartGcodeEdit->toPlainText() != m_savedProfile.pathStartGcode
        || m_pathEndGcodeEdit->toPlainText() != m_savedProfile.pathEndGcode
        || m_layerStartGcodeEdit->toPlainText() != m_savedProfile.layerStartGcode
        || m_layerEndGcodeEdit->toPlainText() != m_savedProfile.layerEndGcode
        || m_interlayerCoolingGcodeEdit->toPlainText() != m_savedProfile.interlayerCoolingGcode
        || m_notesEdit->toPlainText() != m_savedProfile.notes
        || m_currentImageData != m_savedProfile.imageData;
    m_allResetBtn->setIcon(any ? QIcon(":/icons/reset") : QIcon());
    m_allResetBtn->setEnabled(any);

    // 逐字段 reset 按钮状态
    auto setFieldBtn = [](QPushButton* btn, bool dirty) {
        btn->setIcon(dirty ? QIcon(":/icons/reset") : QIcon());
        btn->setEnabled(dirty);
    };

    setFieldBtn(m_printStartGcodeResetBtn,
        m_printStartGcodeEdit->toPlainText() != m_savedProfile.printStartGcode);
    setFieldBtn(m_printEndGcodeResetBtn,
        m_printEndGcodeEdit->toPlainText() != m_savedProfile.printEndGcode);
    setFieldBtn(m_pathStartGcodeResetBtn,
        m_pathStartGcodeEdit->toPlainText() != m_savedProfile.pathStartGcode);
    setFieldBtn(m_pathEndGcodeResetBtn,
        m_pathEndGcodeEdit->toPlainText() != m_savedProfile.pathEndGcode);
    setFieldBtn(m_layerStartGcodeResetBtn,
        m_layerStartGcodeEdit->toPlainText() != m_savedProfile.layerStartGcode);
    setFieldBtn(m_layerEndGcodeResetBtn,
        m_layerEndGcodeEdit->toPlainText() != m_savedProfile.layerEndGcode);
    setFieldBtn(m_interlayerCoolingGcodeResetBtn,
        m_interlayerCoolingGcodeEdit->toPlainText() != m_savedProfile.interlayerCoolingGcode);

    setFieldBtn(m_notesResetBtn,
        m_notesEdit->toPlainText() != m_savedProfile.notes);
}

/************************** 保存 **************************/
void EditMachineDialog::onMachineSave()
{
    if (m_currentProfileId == 0 || !m_repo) return;

    MachineProfile p = m_repo->getById(m_currentProfileId);
    if (p.id == 0) return;

    collectToProfile(p);
    m_repo->save(p);
    p = m_repo->getById(m_currentProfileId);
    loadFromProfile(p);
    m_machineSelectBtn->setText(p.name);
    refreshMachineList();
}

/************************** 新建 **************************/
void EditMachineDialog::onMachineAdd()
{
    if (!m_repo) return;

    MachineProfile profile;
    if (m_currentProfileId > 0) {
        // 有选中 → 复制当前
        profile = m_repo->copy(m_currentProfileId);
        if (profile.id == 0) return;
    } else {
        // 无选中 → 使用默认值
        profile.name = QStringLiteral("新设备");
        m_repo->save(profile);
    }

    m_currentProfileId = profile.id;
    loadFromProfile(profile);
    refreshMachineList();
}

/************************** 删除 **************************/
void EditMachineDialog::onMachineDelete()
{
    if (m_currentProfileId == 0 || !m_repo) return;

    if (!CustomDialog::confirm(this, QStringLiteral("删除设备"),
            QStringLiteral("确定要删除该设备配置吗？此操作不可撤销。")))
        return;

    m_repo->remove(m_currentProfileId);
    m_currentProfileId = 0;
    m_machineSelectBtn->setText(QString());
    refreshMachineList();

    // 删除后，加载剩余第一条
    auto names = m_repo->listAllName();
    if (!names.isEmpty()) {
        m_currentProfileId = names.first().first;
        MachineProfile profile = m_repo->getById(m_currentProfileId);
        loadFromProfile(profile);
        m_machineSelectBtn->setText(profile.name);
    }
}

/************************** 重置（还原到最后一次保存的版本） **************************/
void EditMachineDialog::onMachineReset()
{
    if (m_currentProfileId == 0 || !m_repo) return;

    MachineProfile p = m_repo->getById(m_currentProfileId);
    if (p.id == 0) return;

    loadFromProfile(p);
}
