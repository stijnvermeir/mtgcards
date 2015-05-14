#pragma once

#include <QString>
#include <QVector>
#include <QVariantMap>
#include <QVariant>
#include <QJsonObject>

struct UserColumn
{
	class DataType
	{
	public:
		enum type_t
		{
			String,
			Number,
			Decimal,
			Bool,
			Time,

			COUNT
		};

		DataType(const type_t value = String);
		DataType(const QString& valueString);

		operator QString () const;
		operator type_t () const;

		const QVariant& getEmptyVariant() const;

		static const QVector<DataType>& list();
	private:
		type_t value_;
	};

	DataType dataType_;
	QString name_;

	UserColumn()
		: dataType_()
		, name_() {}

	UserColumn(const DataType& dataType, const QString& name)
		: dataType_(dataType)
		, name_(name) {}

	static QVariantMap loadFromJson(const QJsonObject& obj);
	static void saveToJson(QJsonObject& obj, const QVariantMap& userData);
	static int findIndexOfUserColumnWithName(const QString& name);
};
