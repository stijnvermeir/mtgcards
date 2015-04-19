#pragma once

#include "ui_filtereditordialog.h"
#include "filtermodel.h"

#include <QDialog>

class FilterEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FilterEditorDialog(QWidget *parent = 0);
	~FilterEditorDialog();

private:
	Ui::FilterEditorDialog ui_;
	FilterModel model_;
};
