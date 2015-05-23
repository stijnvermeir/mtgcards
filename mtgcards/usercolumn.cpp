#include "usercolumn.h"

#include "settings.h"

#include <QDateTime>

namespace {

const QVector<QString> NAMES =
{
	"String",
	"Number",
	"Decimal",
	"Bool",
	"Time"
};

const QVector<QVariant> EMPTY_VARIANTS =
{
	QVariant(QVariant::String),
	QVariant(QVariant::Int),
	QVariant(QVariant::Double),
	QVariant(QVariant::Bool),
	QVariant(QVariant::DateTime)
};

} // namespace

UserColumn::DataType::DataType(const type_t value)
	: value_(value)
{
}

UserColumn::DataType::DataType(const QString& stringValue)
	: value_(String)
{
	int index = NAMES.indexOf(stringValue);
	if (index != -1)
	{
		value_ = static_cast<type_t>(index);
	}
}

UserColumn::DataType::operator QString () const
{
	return NAMES[value_];
}

UserColumn::DataType::operator type_t () const
{
	return value_;
}

const QVariant& UserColumn::DataType::getEmptyVariant() const
{
	return EMPTY_VARIANTS[value_];
}

const QVector<UserColumn::DataType>& UserColumn::DataType::list()
{
	static QVector<UserColumn::DataType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(DataType(static_cast<DataType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}

QVariantMap UserColumn::loadFromJson(const QJsonObject& obj)
{
	QVariantMap userData;
	for (const UserColumn& userColumn : Settings::instance().getUserColumns())
	{
		if (obj.contains(userColumn.name_))
		{
			if (userColumn.dataType_ == UserColumn::DataType::String)
			{
				userData[userColumn.name_] = obj[userColumn.name_].toString();
			}
			else
			if (userColumn.dataType_ == UserColumn::DataType::Number)
			{
				userData[userColumn.name_] = obj[userColumn.name_].toInt();
			}
			else
			if (userColumn.dataType_ == UserColumn::DataType::Decimal)
			{
				userData[userColumn.name_] = obj[userColumn.name_].toDouble();
			}
			else
			if (userColumn.dataType_ == UserColumn::DataType::Bool)
			{
				userData[userColumn.name_] = obj[userColumn.name_].toBool();
			}
			else
			if (userColumn.dataType_ == UserColumn::DataType::Time)
			{
				userData[userColumn.name_] = QDateTime::fromString(obj[userColumn.name_].toString(), Qt::ISODate);
			}
		}
	}
	return userData;
}

void UserColumn::saveToJson(QJsonObject& obj, const QVariantMap& userData)
{
	for (auto it = userData.cbegin(); it != userData.cend(); ++it)
	{
		if (it.value().type() == QVariant::DateTime)
		{
			obj[it.key()] = it.value().toDateTime().toString(Qt::ISODate);
		}
		else
		if (it.value().type() == QVariant::Int)
		{
			obj[it.key()] = it.value().toInt();
		}
		else
		if (it.value().type() == QVariant::Double)
		{
			obj[it.key()] = it.value().toDouble();
		}
		else
		if (it.value().type() == QVariant::Bool)
		{
			obj[it.key()] = it.value().toBool();
		}
		else
		{
			obj[it.key()] = it.value().toString();
		}
	}
}

int UserColumn::findIndexOfUserColumnWithName(const QString& name)
{
	for (int index = 0; index < Settings::instance().getUserColumns().size(); ++index)
	{
		if (Settings::instance().getUserColumns()[index].name_ == name)
		{
			return index;
		}
	}
	return -1;
}
