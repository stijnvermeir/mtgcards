#include "optionsdialog.h"
#include "settings.h"
#include "magiccolumntype.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QKeySequenceEdit>
#include <QComboBox>
#include <QProcess>
#include <QDebug>

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

class UserColumnsTableModel : public QAbstractTableModel
{
public:
	QVector<UserColumn> data_;

	struct Column
	{
		enum
		{
			Name,
			Type,
			COUNT
		};
	};

	UserColumnsTableModel()
	{
		data_ = Settings::instance().getUserColumns();
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
			if (role == Qt::DisplayRole || role == Qt::EditRole)
			{
				if (index.row() < rowCount())
				{
					if (index.column() == Column::Name)
					{
						return data_[index.row()].name_;
					}
					if (index.column() == Column::Type)
					{
						return static_cast<QString>(data_[index.row()].dataType_);
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
				if (index.row() < rowCount() && index.column() < Column::COUNT)
				{
					auto & userColumn = data_[index.row()];
					if (index.column() == Column::Name)
					{
						userColumn.name_ = value.toString();
					}
					else
					if (index.column() == Column::Type)
					{
						userColumn.dataType_ = UserColumn::DataType(value.toString());
					}
					emit dataChanged(index, index);
					return true;
				}
			}
		}
		return false;
	}

	virtual bool insertRows(int row, int, const QModelIndex& parent)
	{
		qDebug() << "Insert row at " << row;
		beginInsertRows(parent, row + 1, row + 1);
		data_.insert(row, UserColumn());
		endInsertRows();
		return true;
	}

	virtual bool removeRows(int row, int, const QModelIndex& parent)
	{
		qDebug() << "Remove row at " << row;
		if (row >= 0 && row < data_.size())
		{
			beginRemoveRows(parent, row, row);
			data_.removeAt(row);
			endRemoveRows();
			return true;
		}
		return false;
	}

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal)
		{
			if (role == Qt::DisplayRole)
			{
				if (section == Column::Name)
				{
					return "Name";
				}
				if (section == Column::Type)
				{
					return "Type";
				}
			}
			if (role == Qt::SizeHintRole)
			{
				if (section == Column::Name)
				{
					return QSize(220, 22);
				}
				if (section == Column::Type)
				{
					return QSize(110, 22);
				}
			}
		}
		return QVariant();
	}

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
	}
};

class UserColumnsItemDelegate : public QStyledItemDelegate
{
public:
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column() == UserColumnsTableModel::Column::Type)
		{
			QComboBox* cbx = new QComboBox(parent);
			for (const UserColumn::DataType& type : UserColumn::DataType::list())
			{
				cbx->addItem(static_cast<QString>(type));
			}
			return cbx;
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		if (index.column() == UserColumnsTableModel::Column::Type)
		{
			static_cast<QComboBox*>(editor)->setCurrentText(index.data(Qt::EditRole).toString());
		}
		else
		{
			QStyledItemDelegate::setEditorData(editor, index);
		}
	}

	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		if (index.column() == UserColumnsTableModel::Column::Type)
		{
			model->setData(index, static_cast<QComboBox*>(editor)->currentText());
		}
		else
		{
			QStyledItemDelegate::setModelData(editor, model, index);
		}
	}
};

} // namespace

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, poolReloadRequired_(false)
	, shortcutsModel_(new ShortcutsTableModel())
	, shortcutsItemDelegate_(new ShortcutItemDelegate())
	, userColumnsModel_(new UserColumnsTableModel())
	, userColumnsItemDelegate_(new UserColumnsItemDelegate())
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

	// user columns tab
	ui_.userColumnsTbl->setModel(userColumnsModel_.data());
	ui_.userColumnsTbl->setItemDelegate(userColumnsItemDelegate_.data());
	ui_.userColumnsTbl->resizeColumnsToContents();
	ui_.saveUserColumnsBtn->setEnabled(false);
	connect(userColumnsModel_.data(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(userColumnEdited()));
	connect(ui_.addUserColumnBtn, SIGNAL(released()), this, SLOT(addUserColumnClicked()));
	connect(ui_.delUserColumnBtn, SIGNAL(released()), this, SLOT(delUserColumnClicked()));
	connect(ui_.saveUserColumnsBtn, SIGNAL(released()), this, SLOT(saveUserColumnsClicked()));

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

void OptionsDialog::addUserColumnClicked()
{
	if (userColumnsModel_->insertRow(ui_.userColumnsTbl->currentIndex().row() + 1))
	{
		ui_.saveUserColumnsBtn->setEnabled(true);
	}
}

void OptionsDialog::delUserColumnClicked()
{
	if (userColumnsModel_->removeRow(ui_.userColumnsTbl->currentIndex().row()))
	{
		ui_.saveUserColumnsBtn->setEnabled(true);
	}
}

void OptionsDialog::userColumnEdited()
{
	ui_.saveUserColumnsBtn->setEnabled(true);
}

void OptionsDialog::saveUserColumnsClicked()
{
	const QVector<UserColumn>& userColumns = static_cast<UserColumnsTableModel*>(userColumnsModel_.data())->data_;
	QStringList usedNames;
	for (int i = 0; i < mtg::ColumnType::UserDefined; ++i)
	{
		mtg::ColumnType columnType(static_cast<mtg::ColumnType::type_t>(i));
		usedNames << static_cast<QString>(columnType);
		usedNames << columnType.getDisplayName();
	}
	for (const UserColumn& userColumn : userColumns)
	{
		if (usedNames.contains(userColumn.name_))
		{
			QString msg;
			QTextStream str(&msg);
			str << "Column <i>" << userColumn.name_ << "</i> is not unique or is a name reserved by the application. Please choose a different name for this column.";
			QMessageBox::critical(this, "Invalid user columns", msg);
			return;
		}
		usedNames << userColumn.name_;
	}
	int ret = QMessageBox::question(this,
									"Are you sure?",
									"This will restart the application and all other unsaved changes will be lost. Are you sure you want to continue?",
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::No);
	if (ret == QMessageBox::Yes)
	{
		Settings::instance().setUserColumns(userColumns);
		QProcess::startDetached(QApplication::applicationFilePath());
		QApplication::quit();
	}
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
