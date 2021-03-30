#include "magicitemdelegate.h"
#include "manacost.h"

#include <QPaintEngine>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QCompleter>

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

QWidget* MagicItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	mtg::ColumnType columnType = columnIndexToType(index.column());
	if (columnType == mtg::ColumnType::Quantity || columnType == mtg::ColumnType::Sideboard)
	{
		QSpinBox* sbx = new QSpinBox(parent);
		sbx->setRange(0, 9999);
		return sbx;
	}
	else
	if (columnType == mtg::ColumnType::CMC)
	{
		QSpinBox* sbx = new QSpinBox(parent);
		sbx->setRange(-1, 99);
		sbx->setSpecialValueText("C");
		return sbx;
	}
	else
	if (columnType == mtg::ColumnType::Tags || columnType == mtg::ColumnType::Categories)
	{
		QLineEdit* widget = new QLineEdit(parent);
		QCompleter* completer = new QCompleter(index.data(Qt::EditRole).toStringList(), widget);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		completer->setFilterMode(Qt::MatchContains);
		widget->setCompleter(completer);
		widget->setPlaceholderText("Type to add/remove ...");
		return widget;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void MagicItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	mtg::ColumnType columnType = columnIndexToType(index.column());
	if (columnType == mtg::ColumnType::Quantity || columnType == mtg::ColumnType::Sideboard)
	{
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		if (spinBox)
		{
			spinBox->setValue(index.data().toInt());
		}
	}
	else
	if (columnType == mtg::ColumnType::Tags || columnType == mtg::ColumnType::Categories)
	{
		// empty
	}
	else
	{
		QStyledItemDelegate::setEditorData(editor, index);
	}
}

void MagicItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	mtg::ColumnType columnType = columnIndexToType(index.column());
	if (columnType == mtg::ColumnType::Quantity || columnType == mtg::ColumnType::Sideboard)
	{
		QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
		if (spinBox && model)
		{
			model->setData(index, spinBox->value());
		}
	}
	else
	if (columnType == mtg::ColumnType::Tags || columnType == mtg::ColumnType::Categories)
	{
		QLineEdit* widget = static_cast<QLineEdit*>(editor);
		if (widget && model)
		{
			model->setData(index, widget->text());
		}
	}
	else
	{
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}

mtg::ColumnType MagicItemDelegate::columnIndexToType(const int) const
{
	return mtg::ColumnType::UNKNOWN;
}
