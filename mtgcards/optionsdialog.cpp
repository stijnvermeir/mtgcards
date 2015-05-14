#include "optionsdialog.h"
#include "settings.h"

#include <QFileDialog>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QKeySequenceEdit>

using namespace std;

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

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, poolReloadRequired_(false)
	, shortcutsModel_(new ShortcutsTableModel())
	, shortcutsItemDelegate_(new ShortcutItemDelegate())
{
	ui_.setupUi(this);

	// data bindings tab
	connect(ui_.browseAllSetsJsonBtn, SIGNAL(clicked()), this, SLOT(browseAllSetsJsonBtnClicked()));
	connect(ui_.browseCardPictureDirBtn, SIGNAL(clicked()), this, SLOT(browseCardPictureDirBtnClicked()));
	ui_.allSetsJsonTxt->setText(Settings::instance().getPoolDataFile());
	ui_.cardPictureDirTxt->setText(Settings::instance().getCardImageDir());

	// shortcuts tab
	ui_.shortcutsTbl->setModel(shortcutsModel_.data());
	ui_.shortcutsTbl->setItemDelegate(shortcutsItemDelegate_.data());
	ui_.shortcutsTbl->resizeColumnsToContents();
	ui_.saveShortcutsBtn->setEnabled(false);
	connect(shortcutsModel_.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(shortcutsChanged(QModelIndex,QModelIndex)));
	connect(ui_.saveShortcutsBtn, SIGNAL(released()), this, SLOT(saveShortcutsClicked()));

	// misc tab
	connect(ui_.browseAppDataDirBtn, SIGNAL(clicked()), this, SLOT(browseAppDataDirBtnClicked()));
	ui_.appDataDirTxt->setText(Settings::instance().getAppDataDir());
}

OptionsDialog::~OptionsDialog()
{
}

bool OptionsDialog::isPoolReloadRequired() const
{
	return poolReloadRequired_;
}

void OptionsDialog::browseAllSetsJsonBtnClicked()
{
	QString startDir = ui_.allSetsJsonTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto filename = QFileDialog::getOpenFileName(this, "Locate AllSets.json", startDir, tr("AllSets (*AllSets.json)"));
	if (!filename.isNull())
	{
		ui_.allSetsJsonTxt->setText(filename);
		Settings::instance().setPoolDataFile(filename);
		poolReloadRequired_ = true;
	}
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
