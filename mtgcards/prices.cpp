#include "prices.h"

#include "json.hpp"

#include <fstream>

#include <QDate>
#include <QHash>
#include <QtDebug>

using json = nlohmann::json;

class sax_event_consumer : public json::json_sax_t
{
public:
	enum State
	{
		Init,
		InData,
		InCard,
		InCardFormat,
		InPriceProvider,
		InProviderValues,
		InPrices
	};

	State state_;
	std::string key_;
	std::string cardUuid_;
	std::string cardFormat_;
	std::string priceProvider_;
	std::string priceProviderValue_;
	std::string prices_;
	std::string date_;
	QDate mostRecentDate_;
	float price_;
	QHash<QString, float>* priceList_;

	sax_event_consumer(QHash<QString, float>* priceList)
	    : state_(Init)
	    , key_()
	    , cardUuid_()
	    , cardFormat_()
	    , priceProvider_()
	    , priceProviderValue_()
	    , prices_()
	    , date_()
	    , mostRecentDate_()
	    , price_(0.0f)
	    , priceList_(priceList)
	{
	}

	bool null() override
	{
		return true;
	}

	bool boolean(bool /*val*/) override
	{
		return true;
	}

	bool number_integer(number_integer_t /*val*/) override
	{
		return true;
	}

	bool number_unsigned(number_unsigned_t /*val*/) override
	{
		return true;
	}

	bool number_float(number_float_t val, const string_t& /*s*/) override
	{
		if (cardFormat_ == "paper" && priceProvider_ == "cardmarket" && priceProviderValue_ == "retail" && prices_ == "normal")
		{
			auto d = QDate::fromString(date_.c_str(), Qt::ISODate);
			if (d.isValid())
			{
				if (mostRecentDate_.isNull() || mostRecentDate_ < d)
				{
					mostRecentDate_ = d;
					price_ = val;
				}
			}
		}
		return true;
	}

	bool string(string_t& /*val*/) override
	{
		return true;
	}

	bool start_object(std::size_t /*elements*/) override
	{
		switch (state_)
		{
		case Init:
		{
			if (key_ == "data")
			{
				state_ = InData;
			}
		}
		break;
		case InData:
		{
			cardUuid_ = key_;
			state_ = InCard;
		}
		break;
		case InCard:
		{
			cardFormat_ = key_;
			state_ = InCardFormat;
		}
		break;
		case InCardFormat:
		{
			priceProvider_ = key_;
			state_ = InPriceProvider;
		}
		break;
		case InPriceProvider:
		{
			priceProviderValue_ = key_;
			state_ = InProviderValues;
		}
		break;
		case InProviderValues:
		{
			prices_ = key_;
			state_ = InPrices;
		}
		break;
		case InPrices:
		{

		}
		break;
		}
		return true;
	}

	bool end_object() override
	{
		switch (state_)
		{
		case Init:
		{
		}
		break;
		case InData:
		{
			state_ = Init;
		}
		break;
		case InCard:
		{
			cardUuid_.clear();
			state_ = InData;
		}
		break;
		case InCardFormat:
		{
			cardFormat_.clear();
			state_ = InCard;
		}
		break;
		case InPriceProvider:
		{
			priceProvider_.clear();
			state_ = InCardFormat;
		}
		break;
		case InProviderValues:
		{
			priceProviderValue_.clear();
			state_ = InPriceProvider;
		}
		break;
		case InPrices:
		{
			if (mostRecentDate_.isValid())
			{
				if (priceList_)
				{
					priceList_->insert(cardUuid_.c_str(), price_);
				}
				//qDebug() << cardUuid_.c_str() << mostRecentDate_ << price_;
			}
			mostRecentDate_ = QDate();
			price_ = 0.0f;
			date_.clear();
			prices_.clear();
			state_ = InProviderValues;
		}
		break;
		}
		return true;
	}

	bool start_array(std::size_t /*elements*/) override
	{
		return true;
	}

	bool end_array() override
	{
		return true;
	}

	bool key(string_t& val) override
	{
		if (state_ == InPrices)
		{
			date_ = val;
		}
		else
		{
			key_ = val;
		}
		return true;
	}

	bool binary(json::binary_t& /*val*/) override
	{
		return true;
	}

	bool parse_error(std::size_t /*position*/, const std::string& /*last_token*/, const json::exception& /*ex*/) override
	{
		return false;
	}
};

Prices& Prices::instance()
{
	static Prices prices;
	return prices;
}

Prices::Prices()
{
}

Prices::~Prices()
{
}

void Prices::update(const QString& allPricesJsonFile)
{
	std::ifstream ifs(allPricesJsonFile.toStdString());
	QHash<QString, float> priceList;
	sax_event_consumer sec(&priceList);
	json::sax_parse(ifs, &sec);
}
