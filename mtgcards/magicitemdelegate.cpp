#include "magicitemdelegate.h"

#include "manacost.h"

#include <QPaintEngine>
#include <QLabel>

MagicItemDelegate::MagicItemDelegate(QWidget* parent)
	: QStyledItemDelegate(parent)
{

}

void MagicItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.data().canConvert<ManaCost>())
	{
		ManaCost manaCost = qvariant_cast<ManaCost>(index.data());

		QLabel label;
		QPalette palette = label.palette();
		if (option.state & QStyle::State_Selected)
		{
			palette.setColor(label.backgroundRole(), option.palette.highlight().color());
		}
		else
		{
			palette.setColor(label.backgroundRole(), option.palette.base().color());
		}
		label.setTextFormat(Qt::RichText);
		label.setAlignment(Qt::AlignCenter);
		label.setPalette(palette);
		label.setGeometry(option.rect);
		label.setText(manaCost.getRichText());
		QPixmap pixmap(label.size());
		label.render(&pixmap);

		painter->save();
		painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		painter->setPen(Qt::NoPen);
		painter->drawPixmap(option.rect, pixmap);
		painter->restore();
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
}

