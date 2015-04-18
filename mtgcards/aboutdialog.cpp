#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);
}

AboutDialog::~AboutDialog()
{
}
