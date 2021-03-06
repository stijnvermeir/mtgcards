#include "tableview.h"

#include "magicconvert.h"
#include "magicsortfilterproxymodel.h"
#include "settings.h"

#include <QKeyEvent>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

TableView::TableView(QWidget* parent)
	: QTableView(parent)
	, searchString_()
{
	handleFontChanged();
}

TableView::~TableView()
{
}

void TableView::keyboardSearch(const QString& search)
{
	searchString_ += search;
	if (!findNextHit(currentIndex()))
	{
		searchString_.chop(search.size());
	}
	else
	{
		emit searchStringChanged(searchString_);
	}
}

void TableView::keyPressEvent(QKeyEvent* event)
{
	if (event->matches(QKeySequence::Copy))
	{
		QString text;
		MagicSortFilterProxyModel* mdl = static_cast<MagicSortFilterProxyModel*>(model());
		for (const QModelIndex& row : selectionModel()->selectedRows())
		{
			bool first = true;
			for (const mtg::ColumnType& column : Settings::instance().getCopyColumns())
			{
				int columnIndex = mdl->columnToIndex(column);
				if (columnIndex >= 0 && columnIndex < mdl->columnCount())
				{
					if (first)
					{
						first = false;
					}
					else
					{
						text += '\t';
					}
					text += mtg::toString(mdl->data(mdl->index(row.row(), columnIndex)));
				}
			}
			text += '\n';
		}

		QApplication::clipboard()->setText(text);
		event->accept();
		return;
	}

	if (searchString_.isEmpty())
	{
		QTableView::keyPressEvent(event);
		return;
	}

	if (event->key() == Qt::Key_PageDown)
	{
		QModelIndex current = currentIndex();
		int row = current.row() + 1;
		if (row >= model()->rowCount())
		{
			row = 0;
		}
		findNextHit(model()->index(row, current.column()), true);
		event->accept();
	}
	else
	if (event->key() == Qt::Key_PageUp)
	{
		QModelIndex current = currentIndex();
		int row = current.row() - 1;
		if (row < 0)
		{
			row = model()->rowCount() - 1;
		}
		findNextHit(model()->index(row, current.column()), false);
		event->accept();
	}
	else
	if (event->key() == Qt::Key_Space)
	{
		keyboardSearch(" ");
	}
	else
	if (event->key() == Qt::Key_Backspace)
	{
		searchString_.chop(1);
		emit searchStringChanged(searchString_);
	}
	else
	{
		QTableView::keyPressEvent(event);
	}
}

const QString& TableView::getKeyboardSearchString() const
{
	return searchString_;
}

void TableView::resetKeyboardSearch()
{
	searchString_.clear();
	emit searchStringChanged(searchString_);
}

bool TableView::findNextHit(const QModelIndex& startFrom, const bool searchDown)
{
	int row = startFrom.row();
	int iterations = 0;
	while (iterations < model()->rowCount())
	{
		QModelIndex index = model()->index(row, startFrom.column());
		QString testString = mtg::toString(index.data());
		bool hit = false;
		if (searchString_.startsWith("^"))
		{
			hit = testString.startsWith(searchString_.mid(1), Qt::CaseInsensitive);
		}
		else
		{
			hit = testString.contains(searchString_, Qt::CaseInsensitive);
		}
		if (hit)
		{
			disconnect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChangedSlot()));
			setCurrentIndex(index);
			connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChangedSlot()));
			return true;
		}
		if (searchDown)
		{
			++row;
			if (row >= model()->rowCount())
			{
				row = 0;
			}
		}
		else
		{
			--row;
			if (row < 0)
			{
				row = model()->rowCount() - 1;
			}
		}
		++iterations;
	}
	return false;
}

void TableView::currentChangedSlot()
{
	resetKeyboardSearch();
}

void TableView::handleFontChanged()
{
	setFont(Settings::instance().getFont());
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(Settings::instance().getFont().pointSize() + 15);
}
