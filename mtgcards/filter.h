#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QString>
#include <QJsonObject>
#include <QWidget>

#include <vector>
#include <memory>
#include <functional>

class FilterFunction
{
public:
	typedef std::unique_ptr<FilterFunction> Ptr;

	virtual ~FilterFunction() {}
	virtual const QString& getId() const = 0;
	virtual bool apply(const QVariant& data) const = 0;
	virtual QString getDescription() const = 0;
	virtual QJsonObject toJson() const = 0;
	virtual void fromJson(const QJsonObject& obj) = 0;
	virtual QWidget* createEditor(QWidget* parent) const = 0;
	virtual void updateFromEditor(const QWidget* editor) = 0;
};

class FilterFunctionManager
{
private:
	FilterFunctionManager();
	~FilterFunctionManager();

public:
	typedef std::function<FilterFunction::Ptr(void)> CreateFunc;

	static FilterFunctionManager& instance();

	bool registerFilterFunction(const QString& id, const CreateFunc& createFunc);
	FilterFunction::Ptr createFromId(const QString& id) const;
	const std::vector<QString>& getRegisteredFunctions() const;

private:
	struct Pimpl;
	std::unique_ptr<Pimpl> pimpl_;
};

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
	class Type
	{
	public:
		enum type_t
		{
			AND,
			OR,
			LEAF
		};

		Type(const type_t value = AND);
		Type(const QString& stringValue);

		operator QString() const;
		operator type_t () const;
	private:
		type_t value_;
	};

	typedef std::shared_ptr<FilterNode> Ptr;

	static Ptr create();
	static Ptr createFromFile(const QString& file);

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
