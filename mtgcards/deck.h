#pragma once

#include "magiccolumntype.h"

#include <QVariant>
#include <QString>
#include <QScopedPointer>
#include <QObject>
#include <QVector>
#include <QPair>

class Deck : public QObject
{
	Q_OBJECT

public:
	Deck();
	Deck(const QString& file);
	~Deck();

	void reload();
	void load(const QString& filename);
	void save(const QString& filename);
	const QString& getFilename() const;
	const QString& getId() const;
	QString getDisplayName() const;
	bool hasUnsavedChanges() const;

	int getNumRows() const;
	int getNumCards() const;
	QVariant get(const int row, const mtg::ColumnType& column) const;
	int getDataRowIndex(const int row) const;
	int getRowIndex(const int dataRowIndex) const;

	int getQuantity(const int dataRowIndex) const;
	void setQuantity(const int dataRowIndex, const int newQuantity);

	int getSideboard(const int dataRowIndex) const;
	void setSideboard(const int dataRowIndex, const int newSideboard);

	void overrideManaValue(const int dataRowIndex, QVariant manaValue);

	bool isActive() const;
	void setActive(bool active);

	QVector<QPair<int,int>> getQuantities() const;

	QStringList getCategories(const int dataRowIndex) const;
	QStringList getCategoryCompletions(const int dataRowIndex) const;
	void updateCategories(const int dataRowIndex, const QString& update);

	bool isCommander(const int dataRowIndex) const;
	void setCommander(const int dataRowIndex, bool commander);
	QString getColorIdentity() const;
	bool matchesColorIdentity(const QString& colorId) const;
	bool isLegalForCommander(int row) const;

signals:
	void changed();

private:
	struct Pimpl;
	QScopedPointer<Pimpl> pimpl_;
};
