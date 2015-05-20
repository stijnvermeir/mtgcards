#include "filters/timefilterfunction.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>

using namespace std;

namespace {

const QString ID = "Time";
const bool registered = FilterFunctionManager::instance().registerFilterFunction(ID, []()
{
	return FilterFunction::Ptr(new TimeFilterFunction());
});

struct EditorWidget : public QWidget
{
	QHBoxLayout* layout;
	QComboBox* compareTypeCbx;
	QDateTimeEdit* dateTimeEdt;

	EditorWidget(QWidget* parent)
		: QWidget(parent)
		, layout(new QHBoxLayout(this))
		, compareTypeCbx(new QComboBox(this))
		, dateTimeEdt(new QDateTimeEdit(this))
	{
		for (const auto& item : CompareType::list())
		{
			compareTypeCbx->addItem(static_cast<QString>(item));
		}
		compareTypeCbx->setMaximumWidth(50);
		dateTimeEdt->setDisplayFormat("HH:mm:ss dd.MM.yyyy");
		dateTimeEdt->setCalendarPopup(true);
		layout->setContentsMargins(0, 0, 10, 0);
		layout->addWidget(compareTypeCbx);
		layout->addWidget(dateTimeEdt);
		setLayout(layout);
		setAutoFillBackground(true);
	}
};

} // namespace

TimeFilterFunction::TimeFilterFunction()
	: compareType_()
	, timestamp_(QDate(2000, 1, 1), QTime(0, 0, 0))
{
}

const QString& TimeFilterFunction::getId() const
{
	return ID;
}

bool TimeFilterFunction::apply(const QVariant& data) const
{
	if (data.canConvert<QDateTime>())
	{
		return compareType_.compare(data.toDateTime(), timestamp_);
	}
	return true;
}

QString TimeFilterFunction::getDescription() const
{
	return static_cast<QString>(compareType_) + " " + timestamp_.toString();
}

QJsonObject TimeFilterFunction::toJson() const
{
	QJsonObject obj;
	obj["type"] = ID;
	obj["compareType"] = static_cast<QString>(compareType_);
	obj["timestamp"] = timestamp_.toString("HH:mm:ss dd.MM.yyyy");
	return obj;
}

void TimeFilterFunction::fromJson(const QJsonObject& obj)
{
	compareType_ = obj["compareType"].toString();
	timestamp_ = QDateTime::fromString(obj["timestamp"].toString(), "HH:mm:ss dd.MM.yyyy");
}

QWidget* TimeFilterFunction::createEditor(QWidget* parent) const
{
	EditorWidget* editor = new EditorWidget(parent);
	editor->compareTypeCbx->setCurrentText(static_cast<QString>(compareType_));
	editor->dateTimeEdt->setDateTime(timestamp_);
	return editor;
}

void TimeFilterFunction::updateFromEditor(const QWidget* editor)
{
	const EditorWidget* ed = static_cast<const EditorWidget*>(editor);
	compareType_ = ed->compareTypeCbx->currentText();
	timestamp_ = ed->dateTimeEdt->dateTime();
}
