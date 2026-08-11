#pragma once

#include <QWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QVector>
#include <functional>
#include <QVariant>

class QMenu;

struct Dependent {
    QWidget* widget = nullptr;
    std::function<bool()> condition; // null = 无条件跟随
    bool invert = false;             // true → checked 时隐藏
};

class ParamRow : public QWidget
{
	Q_OBJECT

signals:
	void currentIndexChanged(int index);

public:
	ParamRow(const QString& name, const QString& suffix,
		double min, double max, double step, int decimals, QWidget* parent);

	ParamRow(const QString& name, const QString& suffix,
		int min, int max, int step, QWidget* parent);

	ParamRow(const QString& name, bool check, QWidget* parent);

	ParamRow(const QString& name, const QVector<QString>& list, QWidget* parent);

	~ParamRow() override = default;

	QAbstractSpinBox* spinBox() const { return m_spinBox; }

	QPushButton* undoBtn() const { return m_undoBtn; }

	QCheckBox* checkBox() const { return m_checkBox; }

	QPushButton* combBtn() const { return m_combBtn; }

	QMenu* menu() const { return m_menu; }

	int currentIndex() const { return m_currentIndex; }
	void setCurrentIndex(int index);

	QString currentText() const { return m_combBtn ? m_combBtn->text() : QString(); }

	void addDependent(QWidget* widget, std::function<bool()> condition = nullptr, bool invert = false);
	void refreshDependents();

	void setDbValue(const QVariant& v);
	void revertToSaved();
	bool isModified() const;
	void setModifiedCallback(std::function<void()> cb);

private:
	QVariant currentValue() const;
	void setValueTo(const QVariant& v);
	void updateUndoVisibility();
	void buildParamRow(const QString& name, const QString& suffix);

	void initCommon(const QString& name);

	bool eventFilter(QObject* obj, QEvent* event) override;

	void applyDependents();

	QPushButton* m_undoBtn = nullptr;
	QAbstractSpinBox* m_spinBox = nullptr;
	QWidget* m_inputContainer = nullptr;
	QCheckBox* m_checkBox = nullptr;
	QPushButton* m_combBtn = nullptr;
	QMenu* m_menu = nullptr;
	int m_currentIndex = 0;
	QVector<Dependent> m_dependents;
	QVector<QString> m_items;
	QVariant m_dbValue;
	std::function<void()> m_modifiedCallback;

	bool m_isDouble = false;
};
