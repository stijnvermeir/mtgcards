#include "filter.h"

#include "magicconvert.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include <array>
#include <functional>

using namespace std;

namespace {

const array<QString, FilterFunctionType::COUNT> NAMES =
{{
	"Regex"
}};

FilterNode::Type filterNodeTypeFromString(const QString& s)
{
	if (s == "AND")
	{
		return FilterNode::Type::AND;
	}
	if (s == "OR")
	{
		return FilterNode::Type::OR;
	}
	return FilterNode::Type::LEAF;
}

QString filterNodeTypeToString(const FilterNode::Type t)
{
	if (t == FilterNode::Type::AND)
	{
		return "AND";
	}
	if (t == FilterNode::Type::OR)
	{
		return "OR";
	}
	return "LEAF";
}

QJsonObject filterFunctionToJson(const FilterFunction::Ptr& function)
{
	QJsonObject obj;
	if (function->getType() == FilterFunctionType::Regex)
	{
		obj["type"] = static_cast<QString>(function->getType());
		obj["regex"] = static_cast<const RegexFilterFunction*>(function.get())->getRegex().pattern();
	}
	return obj;
}

FilterFunction::Ptr filterFunctionFromJson(const QJsonObject& obj)
{
	FilterFunctionType  filterFunctionType (obj["type"].toString());
	if (filterFunctionType == FilterFunctionType::Regex)
	{
		return FilterFunctionFactory::createRegex(obj["regex"].toString());
	}
	return FilterFunction::Ptr();
}

} // namespace

// ================================================================
// FilterFunctionType
// ================================================================

FilterFunctionType::FilterFunctionType(const type_t value)
	: value_(value)
{
}

FilterFunctionType::FilterFunctionType(const QString& stringValue)
	: value_(UNKNOWN)
{
	auto it = find(NAMES.begin(), NAMES.end(), stringValue);
	if (it != NAMES.end())
	{
		value_ = static_cast<type_t>(it - NAMES.begin());
	}
}

FilterFunctionType::operator QString () const
{
	if (value_ >= 0 && value_ < COUNT)
	{
		return NAMES[value_];
	}
	return "UNKNOWN";
}

FilterFunctionType::operator type_t () const
{
	return value_;
}

const vector<FilterFunctionType>& FilterFunctionType::list()
{
	static vector<FilterFunctionType> l;
	static bool ready = false;
	if (!ready)
	{
		l.reserve(COUNT);
		for (int i = 0; i < COUNT; ++i)
		{
			l.push_back(FilterFunctionType(static_cast<FilterFunctionType::type_t>(i)));
		}
		ready = true;
	}
	return l;
}

// ================================================================
// RegexFilterFunction
// ================================================================

RegexFilterFunction::RegexFilterFunction()
	: regex_()
{
}

void RegexFilterFunction::setRegex(const QRegularExpression& regex)
{
	regex_ = regex;
}

const QRegularExpression& RegexFilterFunction::getRegex() const
{
	return regex_;
}

FilterFunctionType RegexFilterFunction::getType() const
{
	return FilterFunctionType::Regex;
}

bool RegexFilterFunction::apply(const QVariant& data) const
{
	return regex_.match(mtg::toString(data)).hasMatch();
}

QString RegexFilterFunction::getDescription() const
{
	return regex_.pattern();
}

// ================================================================
// FilterFunctionFactory
// ================================================================

FilterFunction::Ptr FilterFunctionFactory::createRegex(const QString& regexPattern)
{
	auto func = new RegexFilterFunction();
	func->setRegex(QRegularExpression(regexPattern));
	return FilterFunction::Ptr(func);
}

// ================================================================
// FilterNode
// ================================================================

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
	auto doc = QJsonDocument::fromJson(f.readAll());
	QJsonObject obj = doc.object();

	type_ = Type::AND;
	children_.clear();
	filter_.function.reset();

	std::function<void(const QJsonObject&, FilterNode&)> jsonToNode = [&jsonToNode](const QJsonObject& obj, FilterNode& node)
	{
		node.setType(filterNodeTypeFromString(obj["type"].toString()));
		auto childArray = obj["children"].toArray();
		for (const auto& childObj : childArray)
		{
			auto child = create();
			jsonToNode(childObj.toObject(), *child);
			node.addChild(child);
		}
		if (obj.contains("filter"))
		{
			node.filter_.column = mtg::ColumnType(obj["filter"].toObject()["column"].toString());
			node.filter_.function = filterFunctionFromJson(obj["filter"].toObject()["function"].toObject());
		}
	};

	jsonToNode(obj, *this);
	return true;
}

bool FilterNode::saveToFile(const QString& file) const
{
	std::function<QJsonObject(const FilterNode& node)> nodeToJson = [&nodeToJson](const FilterNode& node)
	{
		QJsonObject o;
		o["type"] = filterNodeTypeToString(node.getType());
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
			filterObject["function"] = filterFunctionToJson(node.getFilter().function);
			o["filter"] = filterObject;
		}
		return o;
	};

	QJsonObject obj = nodeToJson(*this);
	QJsonDocument doc(obj);
	QFile f(file);
	if (!f.open(QIODevice::WriteOnly))
	{
		qWarning() << "Failed to save to file " << file;
		return false;
	}
	f.write(doc.toJson());
	return true;
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
	child->parent_ = shared_from_this();
	children_.push_back(child);
}

void FilterNode::removeChild(Ptr& child)
{
	auto it = std::find(children_.begin(), children_.end(), child);
	if (it != children_.end())
	{
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
	filter_ = move(filter);
}
