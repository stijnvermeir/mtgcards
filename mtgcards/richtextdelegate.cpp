#include "richtextdelegate.h"
#include "richtext.h"

#include <QLabel>
#include <QPaintEngine>

void RichTextDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.data().canConvert<RichText>())
	{
		RichText richText = qvariant_cast<RichText>(index.data());

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
		label.setPalette(palette);
		label.setTextFormat(Qt::RichText);
		label.setGeometry(option.rect);
		label.setText(richText.getText());
		label.setAlignment(Qt::AlignCenter);

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
