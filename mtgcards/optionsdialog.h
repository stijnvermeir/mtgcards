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

private slots:
	// data bindings tab
	void browseAllSetsJsonBtnClicked();
	void downloadLatestAllSetsJsonBtnClicked();
	bool confirmRestart();
	void browseCardPictureDirBtnClicked();
	void ultraHighQualityArtClicked(bool enabled);

	// tags tab
	void addTagClicked();
	void removeTagsClicked();

	// categories tab
	void addCategoryClicked();
	void removeCategoriesClicked();

	// shortcuts tab
	void shortcutsChanged(QModelIndex, QModelIndex);
	void saveShortcutsClicked();

	// misc tab
	void browseAppDataDirBtnClicked();
	void changeFontBtnClicked();
	void poolViewButtonsCheckChanged(int state);
	void collectionViewButtonsCheckChanged(int state);
	void deckViewButtonsCheckChanged(int state);
};
