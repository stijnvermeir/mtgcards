#include "optionsdialog.h"
#include "settings.h"

#include <QFileDialog>

OptionsDialog::OptionsDialog(QWidget *parent)
	: QDialog(parent)
	, ui_()
	, poolReloadRequired_(false)
{
	ui_.setupUi(this);

	connect(ui_.browseAllSetsJsonBtn, SIGNAL(clicked()), this, SLOT(browseAllSetsJsonBtnClicked()));
	connect(ui_.browseCardPictureDirBtn, SIGNAL(clicked()), this, SLOT(browseCardPictureDirBtnClicked()));
	connect(ui_.browseAppDataDirBtn, SIGNAL(clicked()), this, SLOT(browseAppDataDirBtnClicked()));

	ui_.allSetsJsonTxt->setText(Settings::instance().getPoolDataFile());
	ui_.cardPictureDirTxt->setText(Settings::instance().getCardImageDir());
	ui_.appDataDirTxt->setText(Settings::instance().getAppDataDir());
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
	QString startDir = ui_.allSetsJsonTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto filename = QFileDialog::getOpenFileName(this, "Locate AllSets.json", startDir, tr("AllSets (*AllSets.json)"));
	if (!filename.isNull())
	{
		ui_.allSetsJsonTxt->setText(filename);
		Settings::instance().setPoolDataFile(filename);
		poolReloadRequired_ = true;
	}
}

void OptionsDialog::browseCardPictureDirBtnClicked()
{
	QString startDir = ui_.cardPictureDirTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto dir = QFileDialog::getExistingDirectory(this, "Locate Card Picture Dir", startDir);
	if (!dir.isNull())
	{
		ui_.cardPictureDirTxt->setText(dir);
		Settings::instance().setCardImageDir(dir);
	}
}

void OptionsDialog::browseAppDataDirBtnClicked()
{
	QString startDir = ui_.appDataDirTxt->text();
	if (startDir.isEmpty())
	{
		startDir = QDir::homePath();
	}
	auto dir = QFileDialog::getExistingDirectory(this, "Locate App Data Dir", startDir);
	if (!dir.isNull())
	{
		ui_.appDataDirTxt->setText(dir);
		Settings::instance().setAppDataDir(dir);
	}
}
