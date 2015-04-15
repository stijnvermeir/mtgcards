#pragma once

#include <QStyledItemDelegate>

class RichTextDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	RichTextDelegate(QWidget* parent = 0);

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
