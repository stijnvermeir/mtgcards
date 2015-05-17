#pragma once

#include "ui_optionsdialog.h"
#include <QDialog>
#include <QScopedPointer>

class QAbstractItemModel;
class QAbstractItemDelegate;

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit OptionsDialog(QWidget *parent = 0);
	~OptionsDialog();

	bool isPoolReloadRequired() const;

private:
	Ui::OptionsDialog ui_;
	bool poolReloadRequired_;
	QScopedPointer<QAbstractItemModel> shortcutsModel_;
	QScopedPointer<QAbstractItemDelegate> shortcutsItemDelegate_;
	QScopedPointer<QAbstractItemModel> userColumnsModel_;
	QScopedPointer<QAbstractItemDelegate> userColumnsItemDelegate_;

private slots:
	// data bindings tab
	void browseAllSetsJsonBtnClicked();
	void browseCardPictureDirBtnClicked();

	// shortcuts tab
	void shortcutsChanged(QModelIndex, QModelIndex);
	void saveShortcutsClicked();

	// user columns tab
	void addUserColumnClicked();
	void delUserColumnClicked();
	void userColumnEdited();
	void saveUserColumnsClicked();

	// misc tab
	void browseAppDataDirBtnClicked();
};
