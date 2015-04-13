#include "optionsdialog.h"

#include <QFileDialog>
#include <QSettings>

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, poolReloadRequired_(false)
{
	ui_.setupUi(this);

	connect(ui_.browseAllSetsJsonBtn, SIGNAL(clicked()), this, SLOT(browseAllSetsJsonBtnClicked()));
	connect(ui_.browseCardPictureDirBtn, SIGNAL(clicked()), this, SLOT(browseCardPictureDirBtnClicked()));

	QSettings settings;
	ui_.allSetsJsonTxt->setText(settings.value("options/datasources/allsetsjson").toString());
	ui_.cardPictureDirTxt->setText(settings.value("options/datasources/cardpicturedir").toString());
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
    auto filename = QFileDialog::getOpenFileName(this, "Locate AllSets.json", "", tr("AllSets (*AllSets.json)"));
	if (!filename.isNull())
	{
		ui_.allSetsJsonTxt->setText(filename);
		QSettings settings;
		settings.setValue("options/datasources/allsetsjson", filename);
		poolReloadRequired_ = true;
	}
}

void OptionsDialog::browseCardPictureDirBtnClicked()
{
	auto dir = QFileDialog::getExistingDirectory(this, "Locate Card Picture Dir");
	if (!dir.isNull())
	{
		ui_.cardPictureDirTxt->setText(dir);
		QSettings settings;
		settings.setValue("options/datasources/cardpicturedir", dir);
	}
}
