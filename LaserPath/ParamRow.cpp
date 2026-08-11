#include "ParamRow.h"
#include "Styles.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QEvent>
#include <QMenu>
#include <QStyle>

namespace {
const QString kParamRowQSS = R"(
QWidget#ParamRow {
	min-width: 340px;
	max-width: 340px;
	min-height: 28px;
	max-height: 28px;
}
QLabel#NameLabel {
	min-width: 180px;
	max-width: 180px;
	min-height: 25px;
	max-height: 25px;
}
QWidget#InputContainer {
	border: 1px solid #ABADAE;
	border-radius: 0px;
	min-width: 120px;
	max-width: 120px;
	min-height: 25px;
	max-height: 25px;
}
QWidget#InputContainer:hover {
	border: 1px solid #00AE42;
}
QWidget#InputContainer[focused="true"] {
	border: 1px solid #00AE42;
}
QAbstractSpinBox#SpinBox {
	color: #000000;
	padding-left: 6px;
	border: none;
	background: transparent;
}
QAbstractSpinBox#SpinBox::up-button,
QAbstractSpinBox#SpinBox::down-button {
	width: 0px;
	border: none;
}
QLabel#SuffixLabel {
	font-size: 10pt;
	color: #ABADAE;
	margin-left: 30px;
}

QWidget#ParamRow QCheckBox {
    spacing: 0px;
    background-color: #2b2b2b;
}

QWidget#ParamRow QCheckBox::indicator {
    border-radius: 0px;
    width: 14px;
    height: 14px;
    border: 1px solid #ABADAE;
    background-color: #FFFFFF;
}

QWidget#ParamRow QCheckBox::indicator:checked {
    border: 1px solid #00AE42;
    background-color: #00AE42;

    image: url(:/icons/checkmark);
}

QWidget#ParamRow QCheckBox::indicator:hover {
    border: 1px solid #00AE42;
}
QPushButton#CombBtn {
	border: 1px solid #ABADAE;
	border-radius: 0px;
	min-width: 108px;
	max-width: 108px;
	min-height: 25px;
	max-height: 25px;
    text-align: left;
	padding-left: 6px;
	padding-right: 6px;
}
QPushButton#CombBtn:hover {
    border: 1px solid #00AE42;
}
		QMenu {
			background: #FFFFFF;
			border: 1px solid #D0D0D0;
			padding: 0px 2px;
		}
		QMenu::item {
			padding: 6px 24px 6px 6px;
			min-width: 150px;
			max-width: 150px;
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
} // namespace

ParamRow::ParamRow(const QString& name, const QString& suffix,
	double min, double max, double step, int decimals, QWidget* parent)
	: QWidget(parent)
	, m_isDouble(true)
{
	auto* spinBox = new QDoubleSpinBox();
	spinBox->setDecimals(decimals);
	spinBox->setSingleStep(step);
	spinBox->setRange(min, max);
	m_spinBox = spinBox;
	buildParamRow(name, suffix);
	connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, [this](){ updateUndoVisibility(); });
}

ParamRow::ParamRow(const QString& name, const QString& suffix,
	int min, int max, int step, QWidget* parent)
	: QWidget(parent)
	, m_isDouble(false)
{
	auto* spinBox = new QSpinBox();
	spinBox->setSingleStep(step);
	spinBox->setRange(min, max);
	m_spinBox = spinBox;
	buildParamRow(name, suffix);
	connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
	        this, [this](){ updateUndoVisibility(); });
}

ParamRow::ParamRow(const QString& name, bool check, QWidget* parent): QWidget(parent)
{
	initCommon(name);

	m_checkBox = new QCheckBox(this);
	m_checkBox->setChecked(check);

	connect(m_checkBox, &QCheckBox::toggled, this, &ParamRow::applyDependents);
	connect(m_checkBox, &QCheckBox::toggled, this, [this](){ updateUndoVisibility(); });

	auto* rowLayout = static_cast<QHBoxLayout*>(layout());
	rowLayout->addWidget(m_checkBox);
	applyDependents();
}

ParamRow::ParamRow(const QString& name, const QVector<QString>& list, QWidget* parent)
	: QWidget(parent)
	, m_items(list)
{
	if (list.isEmpty())
	{
		return;
	}
	initCommon(name);

	m_menu = new QMenu(this);

	for (int i = 0; i < list.size(); ++i)
	{
		const auto& item = list.at(i);
		QAction* action = m_menu->addAction(item);
		action->setCheckable(true);
		connect(action, &QAction::triggered, this, [this, i, item]() {
			m_currentIndex = i;
			m_combBtn->setText(item);
			for (int j = 0; j < m_menu->actions().size(); ++j)
				m_menu->actions().at(j)->setChecked(j == i);
			emit currentIndexChanged(i);
			updateUndoVisibility();
		});
	}
	m_menu->actions().first()->setChecked(true);

	m_combBtn = new QPushButton(this);
	m_combBtn->setObjectName("CombBtn");
	m_combBtn->setIcon(QIcon(":/icons/arrow"));
	m_combBtn->setText(list.first());

	connect(m_combBtn, &QPushButton::clicked, this, [this]() {
		QPoint pos = m_combBtn->mapToGlobal(
			QPoint(0, m_combBtn->height()));
		m_menu->exec(pos);
	});

	auto* rowLayout = static_cast<QHBoxLayout*>(layout());
	rowLayout->addWidget(m_combBtn);
}

void ParamRow::setCurrentIndex(int index)
{
	if (!m_combBtn || index < 0 || index >= m_items.size())
		return;
	m_currentIndex = index;
	m_combBtn->setText(m_items.at(index));
	const auto actions = m_menu->actions();
	for (int j = 0; j < actions.size(); ++j)
		actions.at(j)->setChecked(j == index);
	emit currentIndexChanged(index);
}

void ParamRow::initCommon(const QString& name)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setObjectName("ParamRow");
	setStyleSheet(kParamRowQSS + Styles::kIconBtn);

	auto* rowLayout = new QHBoxLayout(this);
	rowLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	rowLayout->setContentsMargins(10, 0, 0, 0);
	rowLayout->setSpacing(0);

	auto* nameLabel = new QLabel(name, this);
	nameLabel->setObjectName("NameLabel");
	rowLayout->addWidget(nameLabel);

	m_undoBtn = new QPushButton(this);
	m_undoBtn->setIcon(QIcon());
	m_undoBtn->setObjectName("IconBtn");
	m_undoBtn->setToolTip("点击该图标，丢弃当前的修改，恢复到上次保存的数值。");
	m_undoBtn->installEventFilter(this);
	m_undoBtn->setEnabled(false);
	connect(m_undoBtn, &QPushButton::clicked, this, &ParamRow::revertToSaved);
	rowLayout->addWidget(m_undoBtn);

	rowLayout->addSpacing(5);
}

void ParamRow::addDependent(QWidget* widget, std::function<bool()> condition, bool invert)
{
	m_dependents.append({widget, std::move(condition), invert});
	applyDependents();
}

void ParamRow::refreshDependents()
{
	applyDependents();
}

void ParamRow::applyDependents()
{
	if (!m_checkBox) return;
	bool checked = m_checkBox->isChecked();
	for (const auto& d : m_dependents) {
		bool base = d.invert ? !checked : checked;
		bool visible = base && (!d.condition || d.condition());
		d.widget->setVisible(visible);
	}
}

void ParamRow::buildParamRow(const QString& name, const QString& suffix)
{
	initCommon(name);

	m_inputContainer = new QWidget(this);
	m_inputContainer->setObjectName("InputContainer");
	m_inputContainer->setAttribute(Qt::WA_Hover, true);
	m_inputContainer->installEventFilter(this);

	QHBoxLayout* inputLayout = new QHBoxLayout(m_inputContainer);
	inputLayout->setAlignment(Qt::AlignVCenter);
	inputLayout->setContentsMargins(0, 0, 0, 0);
	inputLayout->setSpacing(0);

	m_spinBox->setParent(m_inputContainer);
	m_spinBox->setObjectName("SpinBox");
	m_spinBox->installEventFilter(this);

	QLabel* suffixLabel = new QLabel(suffix, m_inputContainer);
	suffixLabel->setObjectName("SuffixLabel");
	suffixLabel->setAlignment(Qt::AlignCenter);
	inputLayout->addWidget(m_spinBox);
	inputLayout->addWidget(suffixLabel);

	auto* rowLayout = static_cast<QHBoxLayout*>(layout());
	rowLayout->addWidget(m_inputContainer);
}

bool ParamRow::eventFilter(QObject* obj, QEvent* event)
{
	if (m_spinBox && m_inputContainer)
	{
		if (event->type() == QEvent::MouseButtonPress
			&& obj != m_spinBox
			&& (obj == m_inputContainer || obj->parent() == m_inputContainer))
		{
			m_spinBox->setFocus();
			m_spinBox->selectAll();
		}

		if (obj == m_spinBox) {
			if (event->type() == QEvent::FocusIn) {
				m_inputContainer->setProperty("focused", true);
				m_inputContainer->style()->unpolish(m_inputContainer);
				m_inputContainer->style()->polish(m_inputContainer);
			} else if (event->type() == QEvent::FocusOut) {
				m_inputContainer->setProperty("focused", false);
				m_inputContainer->style()->unpolish(m_inputContainer);
				m_inputContainer->style()->polish(m_inputContainer);
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

QVariant ParamRow::currentValue() const
{
	if (m_checkBox)
		return m_checkBox->isChecked();
	if (m_combBtn)
		return m_currentIndex;
	if (m_spinBox) {
		if (auto* d = qobject_cast<QDoubleSpinBox*>(m_spinBox))
			return d->value();
		if (auto* s = qobject_cast<QSpinBox*>(m_spinBox))
			return s->value();
	}
	return {};
}

void ParamRow::setValueTo(const QVariant& v)
{
	if (m_checkBox) {
		m_checkBox->setChecked(v.toBool());
		return;
	}
	if (m_combBtn && v.toInt() >= 0 && v.toInt() < m_items.size()) {
		setCurrentIndex(v.toInt());
		return;
	}
	if (m_spinBox) {
		if (auto* d = qobject_cast<QDoubleSpinBox*>(m_spinBox)) {
			d->setValue(v.toDouble());
			return;
		}
		if (auto* s = qobject_cast<QSpinBox*>(m_spinBox)) {
			s->setValue(v.toInt());
			return;
		}
	}
}

void ParamRow::updateUndoVisibility()
{
	bool modified = m_savedValue.isValid() && currentValue() != m_savedValue;
	m_undoBtn->setIcon(modified ? QIcon(":/icons/reset") : QIcon());
	m_undoBtn->setEnabled(modified);
	if (m_modifiedCallback)
		m_modifiedCallback();
}

void ParamRow::markSaved()
{
	m_savedValue = currentValue();
	m_undoBtn->setIcon(QIcon());
	m_undoBtn->setEnabled(false);
}

void ParamRow::revertToSaved()
{
	if (!m_savedValue.isValid()) return;
	setValueTo(m_savedValue);
}

bool ParamRow::isModified() const
{
	return m_savedValue.isValid() && currentValue() != m_savedValue;
}

void ParamRow::setModifiedCallback(std::function<void()> cb)
{
	m_modifiedCallback = std::move(cb);
}
