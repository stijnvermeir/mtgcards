#pragma once

#include <QTableView>

class TableView : public QTableView
{
	Q_OBJECT

public:
	TableView(QWidget* parent = 0);
	virtual ~TableView();

	virtual void keyboardSearch(const QString& search);
	virtual void keyPressEvent(QKeyEvent* event);

	const QString& getKeyboardSearchString() const;
	void resetKeyboardSearch();

signals:
	void searchStringChanged(const QString& searchString);

private:
	QString searchString_;

	bool findNextHit(const QModelIndex& startFrom, const bool searchDown = true);

private slots:
	void currentChangedSlot();
};
