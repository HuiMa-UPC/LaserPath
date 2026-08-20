#include "SideBarPanel.h"
#include "Styles.h"
#include "SectionHeader.h"
#include "ParamRow.h"
#include "ParamHeader.h"
#include "CustomDialog.h"
#include "database/ProcessRepository.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QScrollArea>
#include <QMenu>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDebug>


SideBarPanel::SideBarPanel(ProcessRepository* processRepo, MachineRepository* machineRepo, QWidget *parent)
	: QWidget(parent)
	, m_processRepo(processRepo)
	, m_machineRepo(machineRepo)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("SideBarPanel");
    setStyleSheet(sideBarPanelQSS);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(createDeviceCard(this));
    layout->addWidget(createProcessCard(this));

    setupModifiedCallbacks();

    // 启动时自动加载第一条工艺数据
    auto names = m_processRepo->listAllName();
    if (!names.isEmpty()) {
        m_currentProfileId = names.first().first;
        ProcessProfile profile = m_processRepo->getById(m_currentProfileId);
        loadFromProfile(profile);
        m_processSelectBtn->setText(profile.name);
    }
}

#pragma region Device

QWidget* SideBarPanel::createDeviceCard(QWidget* parent)
{
    QWidget* content = new QWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_deviceHeader = new SectionHeader(":/icons/device", "设备", ":/icons/settings", content);
    m_deviceContent = createDeviceContent(content);
    layout->addWidget(m_deviceHeader);
    layout->addWidget(m_deviceContent);

    connect(m_deviceHeader, &SectionHeader::clicked, this, [this]() {
        if (m_deviceContent->isHidden())
            m_deviceContent->setHidden(false);
        else
            m_deviceContent->setHidden(true);
        });
    return content;
}

QWidget* SideBarPanel::createDeviceContent(QWidget* parent)
{
    auto* content = new QWidget(parent);

    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(10, 10,
        10, 10);
    contentLayout->setSpacing(0);

#pragma region 设备卡片
    auto* deviceCard = new QWidget(content);
    deviceCard->setObjectName("DeviceCard");
    deviceCard->setStyleSheet(deviceCardQSS);
    deviceCard->setAttribute(Qt::WA_Hover, true);

    auto* cardLayout = new QVBoxLayout(deviceCard);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(0);

    auto* photoLayout = new QHBoxLayout();
    photoLayout->setContentsMargins(0, 0, 0, 0);
    photoLayout->setSpacing(0);
    cardLayout->addLayout(photoLayout);

    QPushButton* plhBtn = new QPushButton(deviceCard);
    plhBtn->setObjectName("IconBtn");
    plhBtn->setStyleSheet(Styles::kIconBtn);
    photoLayout->addWidget(plhBtn, 0, Qt::AlignTop);

    photoLayout->addStretch(1);
    m_deviceImage = new QLabel(deviceCard);
    m_deviceImage->setAlignment(Qt::AlignCenter);
    QPixmap devicePix(":/icons/image");
    devicePix.setDevicePixelRatio(devicePixelRatioF());
    m_deviceImage->setPixmap(devicePix.scaled(
        QSize(140, 140),
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
    photoLayout->addWidget(m_deviceImage);
    photoLayout->addStretch(1);

    m_editBtn = new QPushButton(deviceCard);
    m_editBtn->setObjectName("IconBtn");
    m_editBtn->setStyleSheet(Styles::kIconBtn);
    m_editBtn->setIcon(QIcon(":/icons/edit"));
    photoLayout->addWidget(m_editBtn, 0, Qt::AlignTop);

    m_editMachineDialog = new EditMachineDialog(m_machineRepo, deviceCard);

    connect(m_editBtn, &QPushButton::clicked, this, [this]() {

        // exec() 模态弹出，阻塞
        int ret = m_editMachineDialog->exec();
        if (ret == QDialog::Accepted)
        {
            // 用户点击确定
        }
        else
        {
            // 用户点击取消/关闭窗口
        }
        });

    m_selectBtn = new QPushButton(deviceCard);
    m_selectBtn->setObjectName("SelectBtn");
    m_selectBtn->setIcon(QIcon(":/icons/arrow"));
    m_selectBtn->setText("Rongsu Laser One");
    cardLayout->addWidget(m_selectBtn);

    contentLayout->addWidget(deviceCard);
#pragma endregion

#pragma region 参数区域
    ParamHeader* wireHeader = new ParamHeader(":/icons/wire", "焊丝", content);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(wireHeader);
    contentLayout->addSpacing(5);

    m_diameterRow = new ParamRow("直径", "mm", 0.4, 2.0, 0.1, 1, content);
    m_stickOutRow = new ParamRow("干伸长", "mm", 1, 20, 1, content);

    contentLayout->addWidget(m_diameterRow);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(m_stickOutRow);
    contentLayout->addSpacing(10);
#pragma endregion

    return content;
}

#pragma endregion

#pragma region Process

QWidget* SideBarPanel::createProcessTopBar(QWidget* parent)
{
    QWidget* content = new QWidget(parent);
    content->setObjectName("ProcessTopBar");
    content->setStyleSheet(processTopBar + Styles::kIconBtn);
    QHBoxLayout* layout = new QHBoxLayout(content);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_allResetBtn = new QPushButton(content);
    m_allResetBtn->setIcon(QIcon());
    m_allResetBtn->setToolTip("点击以将所有设置还原到最后一次保存的版本。");
    m_allResetBtn->setObjectName("IconBtn");
    m_allResetBtn->setEnabled(false);
    connect(m_allResetBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentProfileId == 0) return;
        ProcessProfile profile = m_processRepo->getById(m_currentProfileId);
        if (profile.id == 0) return;
        loadFromProfile(profile);
    });

    m_processSelectBtn = new QPushButton(content);
    m_processSelectBtn->setObjectName("SelectBtn");
    m_processSelectBtn->setIcon(QIcon(":/icons/arrow"));
    m_processSelectBtn->setText("Rongsu Laser One Process");
    m_processSelectBtn->setToolTip("切换选择其他工艺参数配置");

    m_processSaveBtn = new QPushButton(content);
    m_processSaveBtn->setIcon(QIcon(":/icons/sideBar_save"));
    m_processSaveBtn->setToolTip("保存工艺配置");
    m_processSaveBtn->setObjectName("IconBtn");
    connect(m_processSaveBtn, &QPushButton::clicked, this, &SideBarPanel::onSaveProcess);

    m_processAddBtn = new QPushButton(content);
    m_processAddBtn->setIcon(QIcon(":/icons/add"));
    m_processAddBtn->setToolTip("新建工艺");
    m_processAddBtn->setObjectName("IconBtn");
    connect(m_processAddBtn, &QPushButton::clicked, this, &SideBarPanel::onNewProcess);

    m_processRenameBtn = new QPushButton(content);
    m_processRenameBtn->setIcon(QIcon(":/icons/nameEdit"));
    m_processRenameBtn->setToolTip("编辑名称");
    m_processRenameBtn->setObjectName("IconBtn");
    connect(m_processRenameBtn, &QPushButton::clicked, this, &SideBarPanel::onRenameProcess);

    m_processDeleteBtn = new QPushButton(content);
    m_processDeleteBtn->setIcon(QIcon(":/icons/delete"));
    m_processDeleteBtn->setToolTip("删除工艺");
    m_processDeleteBtn->setObjectName("IconBtn");
    connect(m_processDeleteBtn, &QPushButton::clicked, this, &SideBarPanel::onDeleteProcess);

    // 工艺选择下拉
    connect(m_processSelectBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "m_processSelectBtn clicked, m_processRepo =" << m_processRepo;
        if (!m_processRepo) return;
        refreshProcessList();
        if (m_processSelectMenu) {
            qDebug() << "showing select menu with" << m_processSelectMenu->actions().size() << "items";
            QPoint pos = m_processSelectBtn->mapToGlobal(
                QPoint(0, m_processSelectBtn->height()));
            m_processSelectMenu->exec(pos);
        } else {
            qDebug() << "m_processSelectMenu is null";
        }
    });

    layout->addWidget(m_allResetBtn);
    layout->addWidget(m_processSelectBtn, 1);
    layout->addWidget(m_processSaveBtn);
    layout->addWidget(m_processAddBtn);
    layout->addWidget(m_processRenameBtn);
    layout->addWidget(m_processDeleteBtn);

    // 首次加载工艺列表
    refreshProcessList();

    return content;
}

QTabBar* SideBarPanel::createProcessTapBar(QWidget* parent)
{
    QTabBar* tabBar = new QTabBar(parent);

    tabBar->addTab(QStringLiteral("策略"));
    tabBar->addTab(QStringLiteral("速度"));
    tabBar->addTab(QStringLiteral("其他"));

    tabBar->setExpanding(false);
    tabBar->setStyleSheet(processTapBar);
    return tabBar;
}

QScrollArea* SideBarPanel::createProcessStack(QWidget* parent)
{
    QScrollArea* scrollArea = new QScrollArea(parent);
    scrollArea->setObjectName("PanelScroll");
    scrollArea->setStyleSheet(panelScrollQSS);

    QStackedWidget* stack = new QStackedWidget(scrollArea);
    stack->addWidget(createStrategyPage(stack));
    stack->addWidget(createSpeedPage(stack));
    // 其他页占位
    stack->addWidget(new QWidget(stack));
    stack->setCurrentIndex(0);

    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(stack);

    return scrollArea;
}

QWidget* SideBarPanel::createStrategyPage(QWidget* parent)
{
    QWidget* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignTop);
    // ── 切片 ──────────────────────────────
    auto* sliceContainer = new QWidget(page);
    auto* sliceLayout = new QVBoxLayout(sliceContainer);
    sliceLayout->setContentsMargins(0, 0, 0, 0);
    sliceLayout->setSpacing(5);

    m_sliceTitle = new ParamHeader(":/icons/layerHeight", "切片", sliceContainer);
    m_layerHeight = new ParamRow("层高", "mm", 0.0, 20.0, 0.1, 1, sliceContainer);
    m_firstLayerHeight = new ParamRow("首层层高", "mm", 0.0, 20.0, 0.1, 1, sliceContainer);
    m_autoRange = new ParamRow("自动切片范围", true, sliceContainer);
    m_zMin = new ParamRow("Z 最小值", "mm", 0.0, 20.0, 0.1, 1, sliceContainer);
    m_zMin->setVisible(false);
    m_zMax = new ParamRow("Z 最大值", "mm", 0.0, 20.0, 0.1, 1, sliceContainer);
    m_zMax->setVisible(false);

    sliceLayout->addWidget(m_sliceTitle);
    sliceLayout->addSpacing(5);
    sliceLayout->addWidget(m_layerHeight);
    sliceLayout->addWidget(m_firstLayerHeight);
    sliceLayout->addWidget(m_autoRange);
    sliceLayout->addWidget(m_zMin);
    sliceLayout->addWidget(m_zMax);

    connect(m_autoRange->checkBox(), &QCheckBox::toggled, this, [this](bool checked) {
        m_zMin->setVisible(!checked);
        m_zMax->setVisible(!checked);
    });

    layout->addWidget(sliceContainer);
    layout->addSpacing(5);

    // ── 精度 ──────────────────────────────
    auto* precisionContainer = new QWidget(page);
    auto* precisionLayout = new QVBoxLayout(precisionContainer);
    precisionLayout->setContentsMargins(0, 0, 0, 0);
    precisionLayout->setSpacing(5);

    m_precisionTitle = new ParamHeader(":/icons/precision", "精度", precisionContainer);
    m_minContourLen = new ParamRow("最小轮廓长度", "mm", 0.0, 20.0, 0.1, 1, precisionContainer);
    m_collinearPrecision = new ParamRow("共线优化精度", "mm", 0.0, 20.0, 0.1, 1, precisionContainer);
    m_gapCloseRadius = new ParamRow("间隙闭合半径", "mm", 0.0, 20.0, 0.1, 1, precisionContainer);

    precisionLayout->addWidget(m_precisionTitle);
    precisionLayout->addSpacing(5);
    precisionLayout->addWidget(m_minContourLen);
    precisionLayout->addWidget(m_collinearPrecision);
    precisionLayout->addWidget(m_gapCloseRadius);

    layout->addWidget(precisionContainer);
    layout->addSpacing(5);

    // ── 偏移 ──────────────────────────────
    auto* offsetContainer = new QWidget(page);
    auto* offsetLayout = new QVBoxLayout(offsetContainer);
    offsetLayout->setContentsMargins(0, 0, 0, 0);
    offsetLayout->setSpacing(5);

    m_offsetTitle = new ParamHeader(":/icons/wall", "偏移", offsetContainer);
    m_enableOffset = new ParamRow("启用偏移", true, offsetContainer);
    m_outerWallWidth = new ParamRow("外墙宽度", "mm", 0.0, 20.0, 0.1, 1, offsetContainer);
    m_innerWallWidth = new ParamRow("内墙宽度", "mm", 0.0, 20.0, 0.1, 1, offsetContainer);
    m_baseContour = new ParamRow("基准轮廓", QVector<QString>{ "全轮廓", "外轮廓", "内轮廓" }, offsetContainer);
    m_autoOffset = new ParamRow("自动偏移", false, offsetContainer);
    m_offsetCount = new ParamRow("偏移数量", "", 0, 99999, 1, offsetContainer);
    m_offsetContourComp = new ParamRow("偏移轮廓补偿", "mm", 0.0, 20.0, 0.1, 1, offsetContainer);
    m_startAngle = new ParamRow("起始角度", "°", 0.0, 360.0, 0.1, 1, offsetContainer);
    m_startPointStrategy = new ParamRow("起始变换策略",
        QVector<QString>{ "固定起点", "按照角度", "按照长度" }, offsetContainer);
    m_angleIncrement = new ParamRow("角度增量", "°", 0.0, 360.0, 0.1, 1, offsetContainer);
    m_lengthIncrement = new ParamRow("长度增量", "mm", 0.0, 360.0, 0.1, 1, offsetContainer);

    m_enableOffset->addDependent(m_outerWallWidth);
    m_enableOffset->addDependent(m_innerWallWidth);
    m_enableOffset->addDependent(m_baseContour);
    m_enableOffset->addDependent(m_autoOffset);
    m_enableOffset->addDependent(m_offsetContourComp);
    m_enableOffset->addDependent(m_startAngle);
    m_enableOffset->addDependent(m_startPointStrategy);
    m_enableOffset->addDependent(m_offsetCount, [this] { return !m_autoOffset->checkBox()->isChecked(); });
    m_enableOffset->addDependent(m_angleIncrement, [this] { return m_startPointStrategy->currentIndex() == 1; });
    m_enableOffset->addDependent(m_lengthIncrement, [this] { return m_startPointStrategy->currentIndex() == 2; });

    connect(m_autoOffset->checkBox(), &QCheckBox::toggled, this, [this]() {
        m_enableOffset->refreshDependents();
    });
    connect(m_startPointStrategy, &ParamRow::currentIndexChanged,
            m_enableOffset, &ParamRow::refreshDependents);

    offsetLayout->addWidget(m_offsetTitle);
    offsetLayout->addSpacing(5);
    offsetLayout->addWidget(m_enableOffset);
    offsetLayout->addWidget(m_outerWallWidth);
    offsetLayout->addWidget(m_innerWallWidth);
    offsetLayout->addWidget(m_baseContour);
    offsetLayout->addWidget(m_autoOffset);
    offsetLayout->addWidget(m_offsetCount);
    offsetLayout->addWidget(m_offsetContourComp);
    offsetLayout->addWidget(m_startAngle);
    offsetLayout->addWidget(m_startPointStrategy);
    offsetLayout->addWidget(m_angleIncrement);
    offsetLayout->addWidget(m_lengthIncrement);

    layout->addWidget(offsetContainer);
    layout->addSpacing(5);

    // 填充容器 — autoOffset 打开时整个填充面板隐藏
    auto* fillContainer = new QWidget(page);
    auto* fillLayout = new QVBoxLayout(fillContainer);
    fillLayout->setContentsMargins(0, 0, 0, 0);
    fillLayout->setSpacing(5);

    m_fillTitle = new ParamHeader(":/icons/infill", "填充", fillContainer);
    m_beadWidth = new ParamRow("焊道宽度", "mm", 0.0, 20.0, 0.1, 1, fillContainer);
    m_fillType = new ParamRow("填充类型", QVector<QString>{ "直线", "往复" }, fillContainer);
    m_adaptiveOverlap = new ParamRow("自适应搭接率", false, fillContainer);
    m_maxOverlapRate = new ParamRow("最小搭接率", "%", 0.0, 100.0, 0.1, 1, fillContainer);
    m_minOverlapRate = new ParamRow("最大搭接率", "%", 0.0, 100.0, 0.1, 1, fillContainer);
    m_overlapRate = new ParamRow("搭接率", "%", 0.0, 100.0, 0.1, 1, fillContainer);

    m_adaptiveOverlap->addDependent(m_maxOverlapRate);
    m_adaptiveOverlap->addDependent(m_minOverlapRate);
    m_adaptiveOverlap->addDependent(m_overlapRate, nullptr, true);

    m_marginStrategy = new ParamRow("首尾边距策略", QVector<QString>{ "自由", "居中" }, fillContainer);
    m_adaptiveOverlap->addDependent(m_marginStrategy, nullptr, true);
    m_scanDirection = new ParamRow("扫描方向", QVector<QString>{ "指定角度", "沿长边", "沿短边" }, fillContainer);
    m_scanAngle = new ParamRow("扫描角度", "°", 0.0, 360.0, 0.1, 1, fillContainer);

    connect(m_scanDirection, &ParamRow::currentIndexChanged, this, [this](int index) {
        m_scanAngle->setVisible(index == 0);
    });

    m_interlayerRotation = new ParamRow("层间旋转角度", "°", 0.0, 360.0, 0.1, 1, fillContainer);
    m_fillAreaComp = new ParamRow("填充区域补偿", "mm", 0.0, 360.0, 0.1, 1, fillContainer);
    m_endExtension = new ParamRow("端部延申", "mm", 0.0, 360.0, 0.1, 1, fillContainer);

    fillLayout->addWidget(m_fillTitle);
    fillLayout->addSpacing(5);
    fillLayout->addWidget(m_beadWidth);
    fillLayout->addWidget(m_fillType);
    fillLayout->addWidget(m_adaptiveOverlap);
    fillLayout->addWidget(m_maxOverlapRate);
    fillLayout->addWidget(m_minOverlapRate);
    fillLayout->addWidget(m_overlapRate);
    fillLayout->addWidget(m_marginStrategy);
    fillLayout->addWidget(m_scanDirection);
    fillLayout->addWidget(m_scanAngle);
    fillLayout->addWidget(m_interlayerRotation);
    fillLayout->addWidget(m_fillAreaComp);
    fillLayout->addWidget(m_endExtension);
    fillLayout->addSpacing(20);

    layout->addWidget(fillContainer);

    // 填充面板：仅在启用偏移 && 自动偏移 同时打开时隐藏
    auto updateFillVisible = [this, fillContainer] {
        bool hide = m_enableOffset->checkBox()->isChecked()
                 && m_autoOffset->checkBox()->isChecked();
        fillContainer->setVisible(!hide);
    };
    connect(m_enableOffset->checkBox(), &QCheckBox::toggled, this, updateFillVisible);
    connect(m_autoOffset->checkBox(), &QCheckBox::toggled, this, updateFillVisible);

    return page;
}

QWidget* SideBarPanel::createSpeedPage(QWidget* parent)
{
    QWidget* page = new QWidget(parent);

    return page;
}

QWidget* SideBarPanel::createProcessCard(QWidget* parent)
{
    QWidget* content = new QWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_processHeader = new SectionHeader(":/icons/process", "工艺", "", content);
    m_processContent = createProcessContent(content);
    layout->addWidget(m_processHeader);
    layout->addWidget(m_processContent);

    connect(m_processHeader, &SectionHeader::clicked, this, [this]() {
        if (m_processContent->isHidden())
            m_processContent->setHidden(false);
        else
            m_processContent->setHidden(true);
        });
    return content;
}

QWidget* SideBarPanel::createProcessContent(QWidget* parent)
{
    auto* content = new QWidget(parent);
    content->setAttribute(Qt::WA_StyledBackground, true);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(10, 10,
        10, 10);
    contentLayout->setSpacing(0);

    QWidget* topBar = createProcessTopBar(content);
    m_tapBar = createProcessTapBar(content);
    QScrollArea* stack = createProcessStack(content);
    contentLayout->addWidget(topBar);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_tapBar);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(stack);

    return content;
}

#pragma endregion

/************************** 工艺相关 **************************/

void SideBarPanel::refreshProcessList()
{
    auto names = m_processRepo->listAllName();

    delete m_processSelectMenu;
    m_processSelectMenu = new QMenu(m_processSelectBtn);

    if (names.isEmpty()) {
        m_processSelectBtn->setText(QStringLiteral("(无工艺)"));
        m_processSelectMenu->addAction(QStringLiteral("(无工艺，请新建)"))->setEnabled(false);
        m_currentProfileId = 0;
        return;
    }

    for (const auto& pair : names) {
        QAction* act = m_processSelectMenu->addAction(pair.second);
        act->setCheckable(true);
        if (pair.first == m_currentProfileId)
            act->setChecked(true);
        connect(act, &QAction::triggered, this, [this, id = pair.first, name = pair.second]() {
            m_currentProfileId = id;
            m_processSelectBtn->setText(name);
            const auto actions = m_processSelectMenu->actions();
            for (int j = 0; j < actions.size(); ++j)
                actions.at(j)->setChecked(j == actions.indexOf(qobject_cast<QAction*>(sender())));
            ProcessProfile profile = m_processRepo->getById(id);
            loadFromProfile(profile);
        });
    }

    if (m_currentProfileId > 0) {
        m_processSelectBtn->setText(m_processRepo->getById(m_currentProfileId).name);
    } else {
        m_processSelectBtn->setText(QStringLiteral("选择工艺..."));
    }
}

void SideBarPanel::loadFromProfile(const ProcessProfile& p)
{
    // === 切片 ===
    m_layerHeight->setDbValue(p.layerHeight);
    m_firstLayerHeight->setDbValue(p.firstLayerHeight);
    m_autoRange->setDbValue(p.autoRange);
    m_zMin->setDbValue(p.zMin);
    m_zMax->setDbValue(p.zMax);

    // === 精度 ===
    m_minContourLen->setDbValue(p.minContourLen);
    m_collinearPrecision->setDbValue(p.collinearPrecision);
    m_gapCloseRadius->setDbValue(p.gapCloseRadius);

    // === 偏移 ===
    m_enableOffset->setDbValue(p.enableOffset);
    m_outerWallWidth->setDbValue(p.outerWallWidth);
    m_innerWallWidth->setDbValue(p.innerWallWidth);
    m_baseContour->setDbValue(p.baseContour);
    m_autoOffset->setDbValue(p.autoOffset);
    m_offsetCount->setDbValue(p.offsetCount);
    m_offsetContourComp->setDbValue(p.offsetContourComp);
    m_startAngle->setDbValue(p.startAngle);
    m_startPointStrategy->setDbValue(p.startPointStrategy);
    m_angleIncrement->setDbValue(p.angleIncrement);
    m_lengthIncrement->setDbValue(p.lengthIncrement);

    // === 填充 ===
    m_beadWidth->setDbValue(p.beadWidth);
    m_fillType->setDbValue(p.fillType);
    m_adaptiveOverlap->setDbValue(p.adaptiveOverlap);
    m_maxOverlapRate->setDbValue(p.maxOverlapRate);
    m_minOverlapRate->setDbValue(p.minOverlapRate);
    m_overlapRate->setDbValue(p.overlapRate);
    m_marginStrategy->setDbValue(p.marginStrategy);
    m_scanDirection->setDbValue(p.scanDirection);
    m_scanAngle->setDbValue(p.scanAngle);
    m_interlayerRotation->setDbValue(p.interlayerRotation);
    m_fillAreaComp->setDbValue(p.fillAreaComp);
    m_endExtension->setDbValue(p.endExtension);
}

void SideBarPanel::setupModifiedCallbacks()
{
	auto cb = [this]() { updateAllResetState(); };
	m_diameterRow->setModifiedCallback(cb);
	m_stickOutRow->setModifiedCallback(cb);
	m_layerHeight->setModifiedCallback(cb);
	m_firstLayerHeight->setModifiedCallback(cb);
	m_autoRange->setModifiedCallback(cb);
	m_zMin->setModifiedCallback(cb);
	m_zMax->setModifiedCallback(cb);
	m_minContourLen->setModifiedCallback(cb);
	m_collinearPrecision->setModifiedCallback(cb);
	m_gapCloseRadius->setModifiedCallback(cb);
	m_enableOffset->setModifiedCallback(cb);
	m_outerWallWidth->setModifiedCallback(cb);
	m_innerWallWidth->setModifiedCallback(cb);
	m_baseContour->setModifiedCallback(cb);
	m_autoOffset->setModifiedCallback(cb);
	m_offsetCount->setModifiedCallback(cb);
	m_offsetContourComp->setModifiedCallback(cb);
	m_startAngle->setModifiedCallback(cb);
	m_startPointStrategy->setModifiedCallback(cb);
	m_angleIncrement->setModifiedCallback(cb);
	m_lengthIncrement->setModifiedCallback(cb);
	m_beadWidth->setModifiedCallback(cb);
	m_fillType->setModifiedCallback(cb);
	m_adaptiveOverlap->setModifiedCallback(cb);
	m_maxOverlapRate->setModifiedCallback(cb);
	m_minOverlapRate->setModifiedCallback(cb);
	m_overlapRate->setModifiedCallback(cb);
	m_marginStrategy->setModifiedCallback(cb);
	m_scanDirection->setModifiedCallback(cb);
	m_scanAngle->setModifiedCallback(cb);
	m_interlayerRotation->setModifiedCallback(cb);
	m_fillAreaComp->setModifiedCallback(cb);
	m_endExtension->setModifiedCallback(cb);
}

void SideBarPanel::updateAllResetState()
{
	bool any = m_diameterRow->isModified()
	        || m_stickOutRow->isModified()
	        || m_layerHeight->isModified()
	        || m_firstLayerHeight->isModified()
	        || m_autoRange->isModified()
	        || m_zMin->isModified()
	        || m_zMax->isModified()
	        || m_minContourLen->isModified()
	        || m_collinearPrecision->isModified()
	        || m_gapCloseRadius->isModified()
	        || m_enableOffset->isModified()
	        || m_outerWallWidth->isModified()
	        || m_innerWallWidth->isModified()
	        || m_baseContour->isModified()
	        || m_autoOffset->isModified()
	        || m_offsetCount->isModified()
	        || m_offsetContourComp->isModified()
	        || m_startAngle->isModified()
	        || m_startPointStrategy->isModified()
	        || m_angleIncrement->isModified()
	        || m_lengthIncrement->isModified()
	        || m_beadWidth->isModified()
	        || m_fillType->isModified()
	        || m_adaptiveOverlap->isModified()
	        || m_maxOverlapRate->isModified()
	        || m_minOverlapRate->isModified()
	        || m_overlapRate->isModified()
	        || m_marginStrategy->isModified()
	        || m_scanDirection->isModified()
	        || m_scanAngle->isModified()
	        || m_interlayerRotation->isModified()
	        || m_fillAreaComp->isModified()
	        || m_endExtension->isModified();
	m_allResetBtn->setIcon(any ? QIcon(":/icons/reset") : QIcon());
	m_allResetBtn->setEnabled(any);
}

void SideBarPanel::onNewProcess()
{
    bool ok = false;
    QString name = CustomDialog::getText(this, QStringLiteral("新建工艺"),
        QStringLiteral("请输入名称:"), QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    ProcessProfile profile;
    if (m_currentProfileId > 0) {
        // 有选中 → 复制当前
        profile = m_processRepo->copy(m_currentProfileId);
        if (profile.id == 0) return;
    }
    // 无选中 → 使用默认值（ProcessProfile 各字段已有默认值）

    profile.name = name.trimmed();
    m_processRepo->save(profile);
    m_currentProfileId = profile.id;
    loadFromProfile(profile);
    refreshProcessList();
}

void SideBarPanel::onDeleteProcess()
{
    if (m_currentProfileId == 0) return;

    if (!CustomDialog::confirm(this, QStringLiteral("删除工艺"),
            QStringLiteral("确定要删除该工艺配置吗？此操作不可撤销。")))
        return;

    m_processRepo->remove(m_currentProfileId);
    m_currentProfileId = 0;
    m_processSelectBtn->setText(QString());
    refreshProcessList();
}

void SideBarPanel::onRenameProcess()
{
    if (m_currentProfileId == 0) return;

    ProcessProfile p = m_processRepo->getById(m_currentProfileId);
    if (p.id == 0) return;

    bool ok = false;
    QString name = CustomDialog::getText(this, QStringLiteral("编辑名称"),
        QStringLiteral("请输入新名称:"), p.name, &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    p.name = name.trimmed();
    m_processRepo->save(p);
    refreshProcessList();
}

void SideBarPanel::onSaveProcess()
{
    if (m_currentProfileId == 0) return;

    ProcessProfile p = m_processRepo->getById(m_currentProfileId);
    if (p.id == 0) return;

    // === 切片 ===
    p.layerHeight      = qobject_cast<QDoubleSpinBox*>(m_layerHeight->spinBox())->value();
    p.firstLayerHeight = qobject_cast<QDoubleSpinBox*>(m_firstLayerHeight->spinBox())->value();
    p.autoRange        = m_autoRange->checkBox()->isChecked();
    p.zMin             = qobject_cast<QDoubleSpinBox*>(m_zMin->spinBox())->value();
    p.zMax             = qobject_cast<QDoubleSpinBox*>(m_zMax->spinBox())->value();

    // === 精度 ===
    p.minContourLen      = qobject_cast<QDoubleSpinBox*>(m_minContourLen->spinBox())->value();
    p.collinearPrecision = qobject_cast<QDoubleSpinBox*>(m_collinearPrecision->spinBox())->value();
    p.gapCloseRadius     = qobject_cast<QDoubleSpinBox*>(m_gapCloseRadius->spinBox())->value();

    // === 偏移 ===
    p.enableOffset       = m_enableOffset->checkBox()->isChecked();
    p.outerWallWidth     = qobject_cast<QDoubleSpinBox*>(m_outerWallWidth->spinBox())->value();
    p.innerWallWidth     = qobject_cast<QDoubleSpinBox*>(m_innerWallWidth->spinBox())->value();
    p.baseContour        = m_baseContour->currentIndex();
    p.autoOffset         = m_autoOffset->checkBox()->isChecked();
    p.offsetCount        = qobject_cast<QSpinBox*>(m_offsetCount->spinBox())->value();
    p.offsetContourComp  = qobject_cast<QDoubleSpinBox*>(m_offsetContourComp->spinBox())->value();
    p.startAngle         = qobject_cast<QDoubleSpinBox*>(m_startAngle->spinBox())->value();
    p.startPointStrategy = m_startPointStrategy->currentIndex();
    p.angleIncrement     = qobject_cast<QDoubleSpinBox*>(m_angleIncrement->spinBox())->value();
    p.lengthIncrement    = qobject_cast<QDoubleSpinBox*>(m_lengthIncrement->spinBox())->value();

    // === 填充 ===
    p.beadWidth          = qobject_cast<QDoubleSpinBox*>(m_beadWidth->spinBox())->value();
    p.fillType           = m_fillType->currentIndex();
    p.adaptiveOverlap    = m_adaptiveOverlap->checkBox()->isChecked();
    p.maxOverlapRate     = qobject_cast<QDoubleSpinBox*>(m_maxOverlapRate->spinBox())->value();
    p.minOverlapRate     = qobject_cast<QDoubleSpinBox*>(m_minOverlapRate->spinBox())->value();
    p.overlapRate        = qobject_cast<QDoubleSpinBox*>(m_overlapRate->spinBox())->value();
    p.marginStrategy     = m_marginStrategy->currentIndex();
    p.scanDirection      = m_scanDirection->currentIndex();
    p.scanAngle          = qobject_cast<QDoubleSpinBox*>(m_scanAngle->spinBox())->value();
    p.interlayerRotation = qobject_cast<QDoubleSpinBox*>(m_interlayerRotation->spinBox())->value();
    p.fillAreaComp       = qobject_cast<QDoubleSpinBox*>(m_fillAreaComp->spinBox())->value();
    p.endExtension       = qobject_cast<QDoubleSpinBox*>(m_endExtension->spinBox())->value();

    m_processRepo->save(p);
	p = m_processRepo->getById(m_currentProfileId);
	loadFromProfile(p);
}
