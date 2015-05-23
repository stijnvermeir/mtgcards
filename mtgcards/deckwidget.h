#pragma once

#include "decktablemodel.h"
#include "filter.h"
#include "ui_deckwidget.h"

#include <QWidget>
#include <QVector>
#include <QString>
#include <QScopedPointer>

class QAbstractItemDelegate;
class Deck;

class DeckWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DeckWidget(const QString& filename, QWidget* parent = 0);
	~DeckWidget();

	void reload();
	void save(const QString& filename);
	void setDeckActive(const bool active);
	const Deck& deck() const;

	void setHeaderState(const QString& headerState);
	void setFilterRootNode(const FilterNode::Ptr& node);
	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	const DeckTableModel& model() const;

signals:
	void selectedCardChanged(int);
	void deckEdited();
	void headerStateChangedSignal(QString);
	void searchStringChanged(const QString& searchString);

private:
	Ui::DeckWidget ui_;
	DeckTableModel deckTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	bool headerStateChangedSlotDisabled_;

public slots:
	void resetSearchString();
	void addToDeck(const QVector<int>&);
	void removeFromDeck(const QVector<int>&);

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void dataChanged(QModelIndex, QModelIndex);
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void headerStateChangedSlot();
};
