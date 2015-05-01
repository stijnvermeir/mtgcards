#pragma once

#include <QStyledItemDelegate>

class MagicItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	MagicItemDelegate(QWidget* parent = 0);

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
