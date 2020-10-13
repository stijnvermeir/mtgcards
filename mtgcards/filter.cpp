#include "filter.h"

#include "usercolumn.h"
#include "settings.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QDebug>

using namespace std;

mtg::ColumnType FilterFunction::getColumnOverride() const
{
	// default = no override
	return mtg::ColumnType::UNKNOWN;
}

struct FilterFunctionManager::Pimpl
{
	QMap<QString, CreateFunc> createFunctions;
	QVector<QString> registeredFunctions;

	bool registerFilterFunction(const QString& id, const CreateFunc& createFunc)
	{
		auto it = createFunctions.find(id);
		if (it == createFunctions.end())
		{
			createFunctions[id] = createFunc;
			registeredFunctions.push_back(id);
			return true;
		}
		qWarning() << "Failed to register filter function with ID " << id;
		return false;
	}

	FilterFunction::Ptr createFromId(const QString& id) const
	{
		auto it = createFunctions.find(id);
		if (it != createFunctions.end())
		{
			return it.value()();
		}
		return FilterFunction::Ptr();
	}
};

FilterFunctionManager& FilterFunctionManager::instance()
{
	static FilterFunctionManager inst;
	return inst;
}

FilterFunctionManager::FilterFunctionManager()
	: pimpl_(new Pimpl())
{
}

FilterFunctionManager::~FilterFunctionManager()
{
}

bool FilterFunctionManager::registerFilterFunction(const QString& id, const CreateFunc& createFunc)
{
	return pimpl_->registerFilterFunction(id, createFunc);
}

FilterFunction::Ptr FilterFunctionManager::createFromId(const QString& id) const
{
	return pimpl_->createFromId(id);
}

const QVector<QString>& FilterFunctionManager::getRegisteredFunctions() const
{
	return pimpl_->registeredFunctions;
}

// =================================================================
// FilterNode
// =================================================================

FilterNode::Type::Type(const type_t value)
	: value_(value)
{
}

FilterNode::Type::Type(const QString& stringValue)
	: value_(LEAF)
{
	if (stringValue == "AND")
	{
		value_ = Type::AND;
	}
	else
	if (stringValue == "OR")
	{
		value_ = Type::OR;
	}
}

FilterNode::Type::operator QString() const
{
	if (value_ == Type::AND)
	{
		return "AND";
	}
	if (value_ == Type::OR)
	{
		return "OR";
	}
	return "LEAF";
}

FilterNode::Type::operator type_t() const
{
	return value_;
}

FilterNode::Ptr FilterNode::create()
{
	return make_shared<FilterNode>();
}

FilterNode::Ptr FilterNode::createFromFile(const QString& file)
{
	FilterNode::Ptr node = create();
	node->loadFromFile(file);
	return node;
}

FilterNode::Ptr FilterNode::createFromJson(const QJsonDocument& doc)
{
	FilterNode::Ptr node = create();
	node->loadFromJson(doc);
	return node;
}

FilterNode::FilterNode()
	: type_(Type::LEAF)
	, children_()
	, parent_()
	, filter_()
{
}

bool FilterNode::loadFromFile(const QString& file)
{
	QFile f(file);
	if (!f.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to load from file " << file;
		return false;
	}
	loadFromJson(QJsonDocument::fromJson(f.readAll()));
	return true;
}

bool FilterNode::saveToFile(const QString& file) const
{
	QFile f(file);
	if (!f.open(QIODevice::WriteOnly))
	{
		qWarning() << "Failed to save to file " << file;
		return false;
	}
	f.write(toJson().toJson());
	return true;
}

void FilterNode::loadFromJson(const QJsonDocument& doc)
{
	type_ = Type::AND;
	children_.clear();
	filter_.function.reset();

	std::function<void(const QJsonObject&, FilterNode&)> jsonToNode = [&jsonToNode](const QJsonObject& obj, FilterNode& node)
	{
		node.setType(obj["type"].toString());
		auto childArray = obj["children"].toArray();
		for (const auto childObj : childArray)
		{
			auto child = create();
			jsonToNode(childObj.toObject(), *child);
			node.addChild(child);
		}
		if (obj.contains("filter"))
		{
			QJsonObject filterObj = obj["filter"].toObject();
			node.filter_.column = mtg::ColumnType(filterObj["column"].toString());
			if (node.filter_.column == mtg::ColumnType::UserDefined)
			{
				node.filter_.column.setUserColumnIndex(UserColumn::findIndexOfUserColumnWithName(filterObj["userColumn"].toString()));
			}
			QJsonObject filterFunction = filterObj["function"].toObject();
			node.filter_.function = FilterFunctionManager::instance().createFromId(filterFunction["type"].toString());
			if (node.filter_.function)
			{
				node.filter_.function->fromJson(filterFunction);
			}
			if (filterObj.contains("negate"))
			{
				node.filter_.negate = filterObj["negate"].toBool();
			}
		}
	};
	jsonToNode(doc.object(), *this);
}

QJsonDocument FilterNode::toJson() const
{
	std::function<QJsonObject(const FilterNode& node)> nodeToJson = [&nodeToJson](const FilterNode& node)
	{
		QJsonObject o;
		o["type"] = static_cast<QString>(node.getType());
		QJsonArray childArray;
		for (const FilterNode::Ptr& child : node.getChildren())
		{
			childArray.append(nodeToJson(*child));
		}
		o["children"] = childArray;
		if (node.getFilter().function)
		{
			QJsonObject filterObject;
			filterObject["column"] = static_cast<QString>(node.getFilter().column);
			if (node.getFilter().column == mtg::ColumnType::UserDefined)
			{
				filterObject["userColumn"] = node.getFilter().column.userColumn().name_;
			}
			filterObject["function"] = node.getFilter().function->toJson();
			filterObject["negate"] = node.getFilter().negate;
			o["filter"] = filterObject;
		}
		return o;
	};
	return QJsonDocument(nodeToJson(*this));
}

FilterNode::Type FilterNode::getType() const
{
	return type_;
}

void FilterNode::setType(const Type type)
{
	type_ = type;
}

const vector<FilterNode::Ptr>& FilterNode::getChildren() const
{
	return children_;
}

void FilterNode::addChild(Ptr& child)
{
	if (child)
	{
		child->parent_ = shared_from_this();
		children_.push_back(child);
	}
}

void FilterNode::removeChild(Ptr& child)
{
	auto it = std::find(children_.begin(), children_.end(), child);
	if (it != children_.end())
	{
		(*it)->parent_.reset();
		children_.erase(it);
	}
}

FilterNode::Ptr FilterNode::getParent() const
{
	return parent_.lock();
}

const Filter& FilterNode::getFilter() const
{
	return filter_;
}

Filter& FilterNode::getFilter()
{
	return filter_;
}

void FilterNode::setFilter(Filter filter)
{
	filter_.column = filter.column;
	filter_.function = std::move(filter.function);
}
