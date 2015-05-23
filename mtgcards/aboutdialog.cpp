#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget* parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);
	ui_.versionLbl->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
}

AboutDialog::~AboutDialog()
{
}
