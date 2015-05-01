#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QString>
#include <QRegularExpression>

#include <vector>
#include <memory>

// ================================================================
// FilterFunctionType
// ================================================================

class FilterFunctionType
{
public:
	enum type_t
	{
		Regex,

		COUNT
	};

	FilterFunctionType(const type_t value = Regex);
	FilterFunctionType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;
private:
	type_t value_;
};

// ================================================================
// FilterFunction
// ================================================================

class FilterFunction
{
public:
	typedef std::shared_ptr<FilterFunction> Ptr;

	virtual ~FilterFunction() {}
	virtual FilterFunctionType getType() const = 0;
	virtual bool apply(const QVariant& data) const = 0;
};

class RegexFilterFunction : public FilterFunction
{
public:
	RegexFilterFunction();
	void setRegex(const QRegularExpression& regex);

	virtual FilterFunctionType getType() const;
	virtual bool apply(const QVariant& data) const;
private:
	QRegularExpression regex_;
};

// ================================================================
// FilterFunctionFactory
// ================================================================

class FilterFunctionFactory
{
public:
	static FilterFunction::Ptr createRegex(const QRegularExpression& r);
};

// ================================================================
// FilterNode
// ================================================================

struct Filter
{
	mtg::ColumnType column;
	FilterFunction::Ptr function;
};

class FilterNode : public std::enable_shared_from_this<FilterNode>
{
public:
	typedef std::shared_ptr<FilterNode> Ptr;

	static Ptr create();

	enum class Type
	{
		AND,
		OR,
		LEAF
	};

	FilterNode();

	void loadFromFile(const QString& file);
	void saveToFile(const QString& file) const;

	Type getType() const;
	void setType(const Type type);

	const std::vector<Ptr>& getChildren() const;
	void addChild(Ptr& child);

	Ptr getParent() const;

	const Filter& getFilter() const;
	void setFilter(const Filter& filter);

private:
	Type type_;
	std::vector<Ptr> children_;
	std::weak_ptr<FilterNode> parent_;
	Filter filter_;
};
