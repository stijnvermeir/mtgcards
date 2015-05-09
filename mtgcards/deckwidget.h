#pragma once

#include "decktablemodel.h"
#include "filter.h"
#include "ui_deckwidget.h"

#include <QWidget>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QScopedPointer>

class QAbstractItemDelegate;

class DeckWidget : public QWidget
{
	Q_OBJECT

public:
	explicit DeckWidget(const QString& filename, QWidget *parent = 0);
	~DeckWidget();

	void reload();
	void save(const QString& filename);
	const QString& getFilename() const;
	QString getDisplayName() const;
	bool hasUnsavedChanges() const;
	bool isDeckActive() const;
	void setDeckActive(const bool active);

	void setHeaderState(const QByteArray& headerState);
	void setFilterRootNode(const FilterNode::Ptr& node);
	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	const DeckTableModel& getModel() const;

signals:
	void selectedCardChanged(int);
	void deckEdited();
	void headerStateChangedSignal(QByteArray);

private:
	Ui::DeckWidget ui_;
	DeckTableModel deckTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;

public slots:
	void addToDeck(const QVector<int>&);
	void removeFromDeck(const QVector<int>&);

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void dataChanged(QModelIndex, QModelIndex);
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void headerStateChangedSlot();
};
