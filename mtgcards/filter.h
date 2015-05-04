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

		COUNT,
		UNKNOWN = -1
	};

	FilterFunctionType(const type_t value = UNKNOWN);
	FilterFunctionType(const QString& stringValue);

	operator QString () const;
	operator type_t () const;

	static const std::vector<FilterFunctionType>& list();
private:
	type_t value_;
};

// ================================================================
// FilterFunction
// ================================================================

class FilterFunction
{
public:
	typedef std::unique_ptr<FilterFunction> Ptr;

	virtual ~FilterFunction() {}
	virtual FilterFunctionType getType() const = 0;
	virtual bool apply(const QVariant& data) const = 0;
	virtual QString getDescription() const = 0;
};

class RegexFilterFunction : public FilterFunction
{
public:
	RegexFilterFunction();

	const QRegularExpression& getRegex() const;
	void setRegex(const QRegularExpression& regex);

	virtual FilterFunctionType getType() const;
	virtual bool apply(const QVariant& data) const;
	virtual QString getDescription() const;
private:
	QRegularExpression regex_;
};

// ================================================================
// FilterFunctionFactory
// ================================================================

class FilterFunctionFactory
{
public:
	static FilterFunction::Ptr createRegex(const QString& regexPattern);
};

// ================================================================
// FilterNode
// ================================================================

struct Filter
{
	mtg::ColumnType column;
	FilterFunction::Ptr function;

	Filter()
		: column(mtg::ColumnType::Name)
		, function(nullptr) {}
};

class FilterNode : public std::enable_shared_from_this<FilterNode>
{
public:
	typedef std::shared_ptr<FilterNode> Ptr;

	static Ptr create();
	static Ptr createFromFile(const QString& file);

	enum class Type
	{
		AND,
		OR,
		LEAF
	};

	FilterNode();

	bool loadFromFile(const QString& file);
	bool saveToFile(const QString& file) const;

	Type getType() const;
	void setType(const Type type);

	const std::vector<Ptr>& getChildren() const;
	void addChild(Ptr& child);
	void removeChild(Ptr& child);

	Ptr getParent() const;

	const Filter& getFilter() const;
	Filter& getFilter();
	void setFilter(Filter filter);

private:
	Type type_;
	std::vector<Ptr> children_;
	std::weak_ptr<FilterNode> parent_;
	Filter filter_;
};
