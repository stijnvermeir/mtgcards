#include "prices.h"
#include "settings.h"

#include "json.hpp"

#include <fstream>

#include <QDate>
#include <QHash>
#include <QtSql>
#include <QtDebug>

using json = nlohmann::json;
namespace  {
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
	std::vector<float> priceVector_;
	Prices* instance_;

	sax_event_consumer(Prices* instance)
	    : state_(Init)
	    , key_()
	    , cardUuid_()
	    , cardFormat_()
	    , priceProvider_()
	    , priceProviderValue_()
	    , prices_()
	    , date_()
	    , mostRecentDate_()
	    , priceVector_()
	    , instance_(instance)
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
			priceVector_.push_back(val);
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
			if (!priceVector_.empty())
			{
				if (instance_)
				{
					auto price = std::accumulate(priceVector_.begin(), priceVector_.end(), 0.0f) / priceVector_.size();
					price = std::round(price * 100.0f) / 100.0f;
					instance_->setPrice(cardUuid_.c_str(), price);
				}
			}
			mostRecentDate_ = QDate();
			priceVector_.clear();
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

QSqlDatabase conn()
{
	return QSqlDatabase::database("prices");
}

QSqlError initDb()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "prices");
	db.setDatabaseName(Settings::instance().getPricesDb());
	if (!db.open()) return db.lastError();
	if (!db.tables().contains("dbschema"))
	{
		QString query;
		QSqlQuery q(db);
		if (!q.exec("CREATE TABLE dbschema (version INTEGER PRIMARY KEY)")) return q.lastError();
	}
	int dbVersion = 0;
	{
		QSqlQuery q(db);
		q.exec("SELECT MAX(version) FROM dbschema");
		if (q.next())
		{
			dbVersion = q.value(0).toInt();
		}
		q.finish();
	}
	if (dbVersion == 0)
	{
		QSqlQuery q(db);
		db.transaction();
		if (!q.exec("CREATE TABLE prices(uuid TEXT PRIMARY KEY, price real)")) return q.lastError();
		if (!q.exec("INSERT INTO dbschema(version) VALUES(1)")) return q.lastError();
		db.commit();
	}
	return QSqlError();
}

} // namespace

Prices& Prices::instance()
{
	static Prices prices;
	return prices;
}

Prices::Prices()
    : priceList_()
    , conn_()
{
	QSqlError err = initDb();
	if (err.isValid())
	{
		qDebug() << err;
	}
	conn_ = conn();
	QSqlQuery q(conn_);
	q.exec("PRAGMA journal_mode = WAL");
	q.exec("PRAGMA synchronous = NORMAL");

	selectQuery_ = new QSqlQuery(conn_);
	selectQuery_->prepare("SELECT uuid, price FROM prices");
	insertQuery_ = new QSqlQuery(conn_);
	insertQuery_->prepare("INSERT INTO prices(uuid, price) VALUES(?, ?)");
	updateQuery_ = new QSqlQuery(conn_);
	updateQuery_->prepare("UPDATE prices SET price = ? WHERE uuid = ?");

	selectQuery_->exec();
	while (selectQuery_->next())
	{
		auto uuid = selectQuery_->value(0).toString();
		auto price = selectQuery_->value(1).toFloat();
		priceList_.insert(uuid, price);
	}
	selectQuery_->finish();
}

Prices::~Prices()
{
	delete updateQuery_;
	delete insertQuery_;
	delete selectQuery_;
}

void Prices::update(const QString& allPricesJsonFile)
{
	conn_.transaction();
	std::ifstream ifs(allPricesJsonFile.toStdString());
	sax_event_consumer sec(this);
	json::sax_parse(ifs, &sec);
	conn_.commit();
}

QVariant Prices::getPrice(const QString& uuid)
{
	auto it = priceList_.find(uuid);
	if (it != priceList_.end())
	{
		return it.value();
	}
	return QVariant();
}

void Prices::setPrice(const QString& uuid, float price)
{
	if (getPrice(uuid).isValid())
	{
		updateQuery_->bindValue(0, price);
		updateQuery_->bindValue(1, uuid);
		updateQuery_->exec();
		updateQuery_->finish();
	}
	else
	{
		insertQuery_->bindValue(0, uuid);
		insertQuery_->bindValue(1, price);
		insertQuery_->exec();
		insertQuery_->finish();
	}
	priceList_[uuid] = price;
}
