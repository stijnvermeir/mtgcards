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
	explicit OptionsDialog(QWidget* parent = 0);
	virtual ~OptionsDialog();

signals:
	void fontChanged();

private:
	Ui::OptionsDialog ui_;
	QScopedPointer<QAbstractItemModel> shortcutsModel_;
	QScopedPointer<QAbstractItemDelegate> shortcutsItemDelegate_;
	QScopedPointer<QAbstractItemModel> userColumnsModel_;
	QScopedPointer<QAbstractItemDelegate> userColumnsItemDelegate_;

private slots:
	// data bindings tab
	void browseAllSetsJsonBtnClicked();
	void downloadLatestAllSetsJsonBtnClicked();
	bool confirmRestart();
	void browseCardPictureDirBtnClicked();

	// tags tab
	void addTagClicked();
	void removeTagsClicked();

	// shortcuts tab
	void shortcutsChanged(QModelIndex, QModelIndex);
	void saveShortcutsClicked();

	// user columns tab
	void addUserColumnClicked();
	void delUserColumnClicked();
	void userColumnEdited();
	void saveUserColumnsClicked();

	// magiccardmarket.eu tab
	void mkmEditFinished();

	// misc tab
	void browseAppDataDirBtnClicked();
	void changeFontBtnClicked();
	void automaticArtDownloadClicked(bool enabled);
};
