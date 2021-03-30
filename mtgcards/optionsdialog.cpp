#include "optionsdialog.h"
#include "settings.h"
#include "magiccolumntype.h"
#include "util.h"
#include "tags.h"
#include "categories.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QKeySequenceEdit>
#include <QComboBox>
#include <QProcess>
#include <QFontDialog>
#include <QInputDialog>
#include <QDebug>

namespace {

class ShortcutsTableModel : public QAbstractTableModel
{
public:
	QMap<ShortcutType, QKeySequence> data_;

	struct Column
	{
		enum
		{
			Action,
			Shortcut,
			COUNT
		};
	};

	ShortcutsTableModel()
	{
		data_ = Settings::instance().getShortcuts();
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return data_.size();
	}

	virtual int columnCount(const QModelIndex& = QModelIndex()) const
	{
		return Column::COUNT;
	}

	virtual QVariant data(const QModelIndex& index, int role) const
	{
		if (index.isValid())
		{
			if (role == Qt::DisplayRole)
			{
				if (index.row() < rowCount())
				{
					if (index.column() == Column::Action)
					{
						return ShortcutType(index.row()).getDescription();
					}
					if (index.column() == Column::Shortcut)
					{
						return data_[ShortcutType(index.row())].toString(QKeySequence::NativeText);
					}
				}
			}
		}
		return QVariant();
	}

	virtual bool setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (index.isValid())
		{
			if (role == Qt::EditRole)
			{
				if (index.row() < rowCount() && index.column() == Column::Shortcut)
				{
					data_[ShortcutType(index.row())] = QKeySequence(value.toString());
					emit dataChanged(index, index);
					return true;
				}
			}
		}
		return false;
	}

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal)
		{
			if (role == Qt::DisplayRole)
			{
				if (section == Column::Action)
				{
					return "Action";
				}
				if (section == Column::Shortcut)
				{
					return "Shortcut";
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (index.column() == Column::Shortcut)
		{
			return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
		}
		return QAbstractTableModel::flags(index);
	}
};

class ShortcutItemDelegate : public QStyledItemDelegate
{
public:
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
	{
		return new QKeySequenceEdit(parent);
	}

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		static_cast<QKeySequenceEdit*>(editor)->setKeySequence(QKeySequence(index.data().toString()));
	}

	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		model->setData(index, static_cast<QKeySequenceEdit*>(editor)->keySequence().toString(QKeySequence::NativeText));
	}
};

} // namespace

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent)
	, ui_()
	, shortcutsModel_(new ShortcutsTableModel())
	, shortcutsItemDelegate_(new ShortcutItemDelegate())
{
	ui_.setupUi(this);

	// data bindings tab
	connect(ui_.browseAllSetsJsonBtn, SIGNAL(clicked()), this, SLOT(browseAllSetsJsonBtnClicked()));
	connect(ui_.downloadLatestBtn, SIGNAL(clicked()), this, SLOT(downloadLatestAllSetsJsonBtnClicked()));
	connect(ui_.browseCardPictureDirBtn, SIGNAL(clicked()), this, SLOT(browseCardPictureDirBtnClicked()));
	ui_.allSetsJsonTxt->setText(Settings::instance().getPoolDataFile());
	ui_.cardPictureDirTxt->setText(Settings::instance().getCardImageDir());
	ui_.ultraHQArtChk->setChecked(Settings::instance().getArtIsHighQuality());
	connect(ui_.ultraHQArtChk, SIGNAL(clicked(bool)), this, SLOT(ultraHighQualityArtClicked(bool)));

	// tags tab
	ui_.tagsList->addItems(Tags::instance().getTags());
	ui_.tagsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(ui_.addTagBtn, SIGNAL(clicked()), this, SLOT(addTagClicked()));
	connect(ui_.removeTagsBtn, SIGNAL(clicked()), this, SLOT(removeTagsClicked()));

	// categories tab
	ui_.categoriesList->addItems(Categories::instance().getCategories());
	ui_.categoriesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(ui_.addCategoryBtn, SIGNAL(clicked()), this, SLOT(addCategoryClicked()));
	connect(ui_.removeCategoriesBtn, SIGNAL(clicked()), this, SLOT(removeCategoriesClicked()));

	// shortcuts tab
	ui_.shortcutsTbl->setModel(shortcutsModel_.data());
	ui_.shortcutsTbl->setItemDelegate(shortcutsItemDelegate_.data());
	ui_.shortcutsTbl->resizeColumnsToContents();
	ui_.saveShortcutsBtn->setEnabled(false);
	connect(shortcutsModel_.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(shortcutsChanged(QModelIndex,QModelIndex)));
	connect(ui_.saveShortcutsBtn, SIGNAL(released()), this, SLOT(saveShortcutsClicked()));

	// copy behavior tab
	for (const mtg::ColumnType& copyColumn : Settings::instance().getCopyColumns())
	{
		QListWidgetItem* item = new QListWidgetItem(copyColumn.getDisplayName());
		item->setData(Qt::UserRole, (QString) copyColumn);
		ui_.copyColumnsLst->addItem(item);
	}
	for (const mtg::ColumnType& column : mtg::ColumnType::list())
	{
		if (!Settings::instance().getCopyColumns().contains(column))
		{
			QListWidgetItem* item = new QListWidgetItem(column.getDisplayName());
			item->setData(Qt::UserRole, (QString) column);
			ui_.availableColumnsLst->addItem(item);
		}
	}

	// misc tab
	connect(ui_.browseAppDataDirBtn, SIGNAL(clicked()), this, SLOT(browseAppDataDirBtnClicked()));
	ui_.appDataDirTxt->setText(Settings::instance().getAppDataDir());
	connect(ui_.fontBtn, SIGNAL(clicked()), this, SLOT(changeFontBtnClicked()));
	ui_.fontTxt->setText(Settings::instance().getFont().toString());
	ui_.poolViewButtonsChk->setCheckState(Settings::instance().getPoolViewButtons() ? Qt::Checked : Qt::Unchecked);
	connect(ui_.poolViewButtonsChk, SIGNAL(stateChanged(int)), this, SLOT(poolViewButtonsCheckChanged(int)));
	ui_.collectionViewButtonsChk->setCheckState(Settings::instance().getCollectionViewButtons() ? Qt::Checked : Qt::Unchecked);
	connect(ui_.collectionViewButtonsChk, SIGNAL(stateChanged(int)), this, SLOT(collectionViewButtonsCheckChanged(int)));
}

OptionsDialog::~OptionsDialog()
{
	QVector<mtg::ColumnType> copyColumns;
	for (int i = 0; i < ui_.copyColumnsLst->count(); ++i)
	{
		copyColumns.push_back(ui_.copyColumnsLst->item(i)->data(Qt::UserRole).toString());
	}
	Settings::instance().setCopyColumns(copyColumns);
}

void OptionsDialog::browseAllSetsJsonBtnClicked()
{
	if (!confirmRestart())
		return;

	QString startDir = ui_.allSetsJsonTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto filename = QFileDialog::getOpenFileName(this, "Locate AllPrintings.sqlite", startDir, tr("AllPrintings (*AllPrintings.sqlite)"));
	if (!filename.isNull())
	{
		Settings::instance().setPoolDataFile(filename);
		QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
		QApplication::quit();
	}
}

void OptionsDialog::downloadLatestAllSetsJsonBtnClicked()
{
	if (!confirmRestart())
		return;

	if (Util::downloadPoolDataFile())
	{
		QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
		QApplication::quit();
	}
}

bool OptionsDialog::confirmRestart()
{
	int ret = QMessageBox::warning(this,
									"Restart required",
	                                "A restart is required when changing AllPrintings.sqlite. All unsaved changes will be lost.",
									QMessageBox::Ok | QMessageBox::Cancel,
									QMessageBox::Cancel);
	return ret == QMessageBox::Ok;
}

void OptionsDialog::browseCardPictureDirBtnClicked()
{
	QString startDir = ui_.cardPictureDirTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto dir = QFileDialog::getExistingDirectory(this, "Locate Card Picture Dir", startDir);
	if (!dir.isNull())
	{
		ui_.cardPictureDirTxt->setText(dir);
		Settings::instance().setCardImageDir(dir);
	}
}

void OptionsDialog::ultraHighQualityArtClicked(bool enabled)
{
	Settings::instance().setArtIsHighQuality(enabled);
}

void OptionsDialog::addTagClicked()
{
	QString tag = QInputDialog::getText(this, "New tag", "Tag:");
	if (!tag.isNull())
	{
		Tags::instance().addTag(tag);
		ui_.tagsList->clear();
		ui_.tagsList->addItems(Tags::instance().getTags());
	}
}

void OptionsDialog::removeTagsClicked()
{
	for (QListWidgetItem* item : ui_.tagsList->selectedItems())
	{
		Tags::instance().removeTag(item->text());
	}
	ui_.tagsList->clear();
	ui_.tagsList->addItems(Tags::instance().getTags());
}

void OptionsDialog::addCategoryClicked()
{
	QString cat = QInputDialog::getText(this, "New category", "Category:");
	if (!cat.isNull())
	{
		Categories::instance().addCategory(cat);
		ui_.categoriesList->clear();
		ui_.categoriesList->addItems(Categories::instance().getCategories());
	}
}

void OptionsDialog::removeCategoriesClicked()
{
	for (QListWidgetItem* item : ui_.categoriesList->selectedItems())
	{
		Categories::instance().removeCategory(item->text());
	}
	ui_.categoriesList->clear();
	ui_.categoriesList->addItems(Categories::instance().getCategories());
}

void OptionsDialog::shortcutsChanged(QModelIndex, QModelIndex)
{
	ui_.saveShortcutsBtn->setEnabled(true);
}

void OptionsDialog::saveShortcutsClicked()
{
	Settings::instance().setShortcuts(static_cast<ShortcutsTableModel*>(shortcutsModel_.data())->data_);
	ui_.saveShortcutsBtn->setEnabled(false);
}

void OptionsDialog::browseAppDataDirBtnClicked()
{
	QString startDir = ui_.appDataDirTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto dir = QFileDialog::getExistingDirectory(this, "Locate App Data Dir", startDir);
	if (!dir.isNull())
	{
		ui_.appDataDirTxt->setText(dir);
		Settings::instance().setAppDataDir(dir);
	}
}

void OptionsDialog::changeFontBtnClicked()
{
	bool ok;
	QFont f = QFontDialog::getFont(&ok, Settings::instance().getFont(), this);
	if (ok)
	{
		Settings::instance().setFont(f);
		ui_.fontTxt->setText(Settings::instance().getFont().toString());
		emit fontChanged();
	}
}

void OptionsDialog::poolViewButtonsCheckChanged(int state)
{
	Settings::instance().setPoolViewButtons(state == Qt::Checked);
}

void OptionsDialog::collectionViewButtonsCheckChanged(int state)
{
	Settings::instance().setCollectionViewButtons(state == Qt::Checked);
}

void OptionsDialog::deckViewButtonsCheckChanged(int state)
{
	Settings::instance().setDeckViewButtons(state == Qt::Checked);
}
