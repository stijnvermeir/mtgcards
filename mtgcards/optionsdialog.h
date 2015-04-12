#pragma once

#include "ui_optionsdialog.h"
#include <QDialog>

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

private slots:
	void browseAllSetsJsonBtnClicked();
	void browseCardPictureDirBtnClicked();
};
