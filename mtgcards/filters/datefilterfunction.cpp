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
		for (const auto& item : DateFilterFunction::CompareType::list())
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

DateFilterFunction::CompareType::CompareType(const type_t value)
	: value_(value)
{
}

DateFilterFunction::CompareType::CompareType(const QString& stringValue)
	: value_(GreaterThan)
{
	if (stringValue == ">")
	{
		value_ = GreaterThan;
	}
	else
	if (stringValue == "<")
	{
		value_ = LessThan;
	}
}

DateFilterFunction::CompareType::operator QString () const
{
	if (value_ == GreaterThan)
	{
		return ">";
	}
	if (value_ == LessThan)
	{
		return "<";
	}
	return ">";
}

DateFilterFunction::CompareType::operator type_t () const
{
	return value_;
}

const vector<DateFilterFunction::CompareType>& DateFilterFunction::CompareType::list()
{
	static vector<CompareType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(CompareType(static_cast<CompareType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}

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
		if (compareType_ == CompareType::GreaterThan)
		{
			return data.toDate() > date_;
		}
		else
		if (compareType_ == CompareType::LessThan)
		{
			return data.toDate() < date_;
		}
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
