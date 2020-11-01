#pragma once

#include "decktablemodel.h"
#include "filter.h"
#include "commonactions.h"
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
	explicit DeckWidget(const QString& filename, CommonActions& commonActions, QWidget* parent = 0);
	~DeckWidget();

	void save(const QString& filename);
	void setDeckActive(const bool active);
	const Deck& deck() const;

	void setHeaderState(const QString& headerState);
	void setFilterRootNode(const FilterNode::Ptr& node);
	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	const DeckTableModel& model() const;
	bool isColumnHidden(const mtg::ColumnType& columnType) const;

signals:
	void selectedCardChanged(int);
	void deckEdited();
	void headerStateChangedSignal(QString);
	void searchStringChanged(const QString& searchString);
	void fontChanged();

private:
	Ui::DeckWidget ui_;
	DeckTableModel deckTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	bool headerStateChangedSlotDisabled_;
	CommonActions& commonActions_;

public slots:
	void resetSearchString();
	void addToDeck(const QVector<int>&);
	void removeFromDeck(const QVector<int>&);
	void downloadCardArt();
	void fetchOnlineData();

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void dataChanged(QModelIndex, QModelIndex);
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void rowContextMenuRequested(const QPoint& pos);
	void headerStateChangedSlot();
};
