#pragma once

#include "pooltablemodel.h"
#include "imagetablemodel.h"
#include "filter.h"
#include "commonactions.h"
#include "ui_mainwindow.h"

#include <QVector>
#include <QScopedPointer>

class PoolDock : public QObject
{
	Q_OBJECT

public:
	explicit PoolDock(Ui::MainWindow& ui, QObject* parent = nullptr);
	virtual ~PoolDock();

	void updateShortcuts();
	void loadSettings();
	void saveSettings();

signals:
	void selectedCardChanged(int);
	void addToCollection(QVector<int>);
	void removeFromCollection(QVector<int>);
	void addToDeck(QVector<int>);
	void removeFromDeck(QVector<int>);
	void fontChanged();

private:
	Ui::MainWindow& ui_;
	PoolTableModel poolTableModel_;
	ImageTableModel imageTableModel_;
	QScopedPointer<QAbstractItemDelegate> itemDelegate_;
	FilterNode::Ptr rootFilterNode_;
	CommonActions commonActions_;

	int currentDataRowIndex() const;
	QVector<int> currentDataRowIndices() const;
	void updateStatusBar();

private slots:
	void currentRowChanged(QModelIndex, QModelIndex);
	void actionAdvancedFilter();
	void actionEnableFilter(bool enable);
	void actionAddToCollection();
	void actionRemoveFromCollection();
	void actionAddToDeck();
	void actionRemoveFromDeck();
	void actionDownloadCardArt();
	void actionFetchOnlineData();
	void hideColumnsContextMenuRequested(const QPoint& pos);
	void rowContextMenuRequested(const QPoint& pos);
	void handleGlobalFilterChanged();
	void statusBarViewChanged(int index);
	void imageCurrentChanged(const QModelIndex&, const QModelIndex&);
};
