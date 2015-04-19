#include "filtereditordialog.h"

FilterEditorDialog::FilterEditorDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, model_()
{
	ui_.setupUi(this);
	ui_.treeView->setModel(&model_);
}

FilterEditorDialog::~FilterEditorDialog()
{
}
