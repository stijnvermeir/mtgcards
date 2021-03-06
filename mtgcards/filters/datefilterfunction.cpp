#include "filters/datefilterfunction.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>

using namespace std;

namespace {

const QString ID = "Date";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new DateFilterFunction());
});

struct EditorWidget : public QWidget
{
	QHBoxLayout* layout;
	QComboBox* compareTypeCbx;
	QDateEdit* dateEdt;

	EditorWidget(QWidget* parent)
		: QWidget(parent)
		, layout(new QHBoxLayout(this))
		, compareTypeCbx(new QComboBox(this))
		, dateEdt(new QDateEdit(this))
	{
		for (const auto& item : CompareType::list())
		{
			compareTypeCbx->addItem(static_cast<QString>(item));
		}
		compareTypeCbx->setMaximumWidth(50);
		dateEdt->setDisplayFormat("dd.MM.yyyy");
		dateEdt->setCalendarPopup(true);
		layout->setContentsMargins(0, 0, 10, 0);
		layout->addWidget(compareTypeCbx);
		layout->addWidget(dateEdt);
		setLayout(layout);
		setAutoFillBackground(true);
	}
};

} // namespace

DateFilterFunction::DateFilterFunction()
	: compareType_()
	, date_(2000, 1, 1)
{
}

const QString& DateFilterFunction::getId() const
{
	return ID;
}

bool DateFilterFunction::apply(const QVariant& data) const
{
	if (data.canConvert<QDate>())
	{
		return compareType_.compare(data.toDate(), date_);
	}
	return true;
}

QString DateFilterFunction::getDescription() const
{
	return static_cast<QString>(compareType_) + " " + date_.toString();
}

QJsonObject DateFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["compareType"] = static_cast<QString>(compareType_);
	obj["date"] = date_.toString("dd.MM.yyyy");
	return obj;
}

void DateFilterFunction::fromJson(const QJsonObject& obj)
{
	compareType_ = obj["compareType"].toString();
	date_ = QDate::fromString(obj["date"].toString(), "dd.MM.yyyy");
}

QWidget* DateFilterFunction::createEditor(QWidget* parent) const
{
	EditorWidget* editor = new EditorWidget(parent);
	editor->compareTypeCbx->setCurrentText(static_cast<QString>(compareType_));
	editor->dateEdt->setDate(date_);
	return editor;
}

void DateFilterFunction::updateFromEditor(const QWidget* editor)
{
	const EditorWidget* ed = static_cast<const EditorWidget*>(editor);
	compareType_ = ed->compareTypeCbx->currentText();
	date_ = ed->dateEdt->date();
}
