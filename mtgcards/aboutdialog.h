#pragma once

#include "ui_aboutdialog.h"
#include <QDialog>

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget* parent = 0);
	~AboutDialog();

private:
	Ui::AboutDialog ui_;
};
