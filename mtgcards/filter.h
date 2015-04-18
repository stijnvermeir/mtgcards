#pragma once

#include <QVariant>

#include <list>
#include <functional>
#include <memory>

typedef std::function<bool(QVariant)> FilterFunction;

struct Filter
{
	int column;
	FilterFunction function;
};

struct FilterNode
{
	typedef std::unique_ptr<FilterNode> Ptr;

	static Ptr create()
	{
		return Ptr(new FilterNode());
	}

	enum class Type
	{
		AND,
		OR,
		LEAF,
		NO_FILTER
	};

	Type type;
	std::list<Ptr> children;
	Filter filter;

	FilterNode()
		: type(Type::NO_FILTER)
		, children()
		, filter()
	{
	}
};
