#include "filter.h"

#include "magicconvert.h"

#include <array>

using namespace std;

namespace {

const array<QString, FilterFunctionType::COUNT> NAMES =
{{
	"Regex"
}};

} // namespace

// ================================================================
// FilterFunctionType
// ================================================================

FilterFunctionType::FilterFunctionType(const type_t value)
	: value_(value)
{
}

FilterFunctionType::FilterFunctionType(const QString& stringValue)
	: value_(Regex)
{
	auto it = find(NAMES.begin(), NAMES.end(), stringValue);
	if (it != NAMES.end())
	{
		value_ = static_cast<type_t>(it - NAMES.begin());
	}
}

FilterFunctionType::operator QString () const
{
	return NAMES[value_];
}

FilterFunctionType::operator type_t () const
{
	return value_;
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

FilterFunctionType RegexFilterFunction::getType() const
{
	return FilterFunctionType::Regex;
}

bool RegexFilterFunction::apply(const QVariant& data) const
{
	return regex_.match(mtg::toString(data)).hasMatch();
}

// ================================================================
// FilterFunctionFactory
// ================================================================

FilterFunction::Ptr FilterFunctionFactory::createRegex(const QRegularExpression& r)
{
	auto func = new RegexFilterFunction();
	func->setRegex(r);
	return FilterFunction::Ptr(func);
}

// ================================================================
// FilterNode
// ================================================================

FilterNode::Ptr FilterNode::create()
{
	return make_shared<FilterNode>();
}

FilterNode::FilterNode()
	: type_(Type::LEAF)
	, children_()
	, parent_()
	, filter_()
{
}

void FilterNode::loadFromFile(const QString& /*file*/)
{
	// TODO
}

void FilterNode::saveToFile(const QString& /*file*/) const
{
	// TODO
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

FilterNode::Ptr FilterNode::getParent() const
{
	return parent_.lock();
}

const Filter& FilterNode::getFilter() const
{
	return filter_;
}

void FilterNode::setFilter(const Filter& filter)
{
	filter_ = filter;
}
