#pragma once

#include <QString>
#include <QVariant>
#include <QHash>
#include <QtSql>

class Prices
{
public:
	static Prices& instance();

	void update(const QString& allPricesJsonFile);
	QVariant getPrice(const QString& uuid);
	void setPrice(const QString& uuid, float price);
private:
	Prices();
	~Prices();

	QHash<QString, float> priceList_;
	QSqlDatabase conn_;
	QSqlQuery* selectQuery_;
	QSqlQuery* insertQuery_;
	QSqlQuery* updateQuery_;
};

