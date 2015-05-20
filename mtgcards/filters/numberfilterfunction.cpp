#include "filters/numberfilterfunction.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>

using namespace std;

namespace {

const QString ID = "Number";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new NumberFilterFunction());
});

struct EditorWidget : public QWidget
{
	QHBoxLayout* layout;
	QComboBox* compareTypeCbx;
	QDoubleSpinBox* spinBox;

	EditorWidget(QWidget* parent)
		: QWidget(parent)
		, layout(new QHBoxLayout(this))
		, compareTypeCbx(new QComboBox(this))
		, spinBox(new QDoubleSpinBox(this))
	{
		for (const auto& item : CompareType::list())
		{
			compareTypeCbx->addItem(static_cast<QString>(item));
		}
		compareTypeCbx->setMaximumWidth(50);
		layout->setContentsMargins(0, 0, 10, 0);
		layout->addWidget(compareTypeCbx);
		layout->addWidget(spinBox);
		setLayout(layout);
		setAutoFillBackground(true);
	}
};

} // namespace

NumberFilterFunction::NumberFilterFunction()
	: compareType_()
	, number_(0.0)
{
}

const QString& NumberFilterFunction::getId() const
{
	return ID;
}

bool NumberFilterFunction::apply(const QVariant& data) const
{
	if (data.canConvert<double>())
	{
		return compareType_.compare(data.toDouble(), number_);
	}
	return true;
}

QString NumberFilterFunction::getDescription() const
{
	return static_cast<QString>(compareType_) + " " + QString::number(number_);
}

QJsonObject NumberFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["compareType"] = static_cast<QString>(compareType_);
	obj["number"] = number_;
	return obj;
}

void NumberFilterFunction::fromJson(const QJsonObject& obj)
{
	compareType_ = obj["compareType"].toString();
	number_ = obj["number"].toDouble();
}

QWidget* NumberFilterFunction::createEditor(QWidget* parent) const
{
	EditorWidget* editor = new EditorWidget(parent);
	editor->compareTypeCbx->setCurrentText(static_cast<QString>(compareType_));
	editor->spinBox->setValue(number_);
	return editor;
}

void NumberFilterFunction::updateFromEditor(const QWidget* editor)
{
	const EditorWidget* ed = static_cast<const EditorWidget*>(editor);
	compareType_ = ed->compareTypeCbx->currentText();
	number_ = ed->spinBox->value();
}
