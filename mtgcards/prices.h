#pragma once

#include <QString>

class Prices
{
public:
	static Prices& instance();

	void update(const QString& allPricesJsonFile);
private:
	Prices();
	~Prices();
};

