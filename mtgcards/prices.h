#pragma once

#include <QString>
#include <QVariant>
#include <QHash>

class Prices
{
public:
	static Prices& instance();

	void update(const QString& allPricesJsonFile);
	QVariant getPrice(const QString& uuid) const;
	void setPrice(const QString& uuid, float price);
private:
	Prices();
	~Prices();

	QHash<QString, float> priceList_;
};

