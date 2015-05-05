#include "mainwindow.h"

#include "optionsdialog.h"
#include "aboutdialog.h"

#include <QDebug>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolWindow_()
	, cardWindow_()
	, collectionWindow_()
	, deckWindow_()
{
	ui_.setupUi(this);

	qRegisterMetaType<mtg::LayoutType>();

	loadSettings();

	// window management
	ui_.actionPoolWindow->setChecked(poolWindow_.isVisible());
	connect(ui_.actionPoolWindow, SIGNAL(toggled(bool)), this, SLOT(poolWindowActionToggled(bool)));
	connect(&poolWindow_, SIGNAL(windowClosed(bool)), ui_.actionPoolWindow, SLOT(setChecked(bool)));
	ui_.actionCardWindow->setChecked(cardWindow_.isVisible());
	connect(ui_.actionCardWindow, SIGNAL(toggled(bool)), this, SLOT(cardWindowActionToggled(bool)));
	connect(&cardWindow_, SIGNAL(windowClosed(bool)), ui_.actionCardWindow, SLOT(setChecked(bool)));
	ui_.actionCollectionWindow->setChecked(collectionWindow_.isVisible());
	connect(ui_.actionCollectionWindow, SIGNAL(toggled(bool)), this, SLOT(collectionWindowActionToggled(bool)));
	connect(&collectionWindow_, SIGNAL(windowClosed(bool)), ui_.actionCollectionWindow, SLOT(setChecked(bool)));
	ui_.actionDeckWindow->setChecked(deckWindow_.isVisible());
	connect(ui_.actionDeckWindow, SIGNAL(toggled(bool)), this, SLOT(deckWindowActionToggled(bool)));
	connect(&deckWindow_, SIGNAL(windowClosed(bool)), ui_.actionDeckWindow, SLOT(setChecked(bool)));

	// options
	connect(ui_.actionOptions, SIGNAL(triggered()), this, SLOT(optionsActionClicked()));

	// about
	connect(ui_.actionAbout, SIGNAL(triggered()), this, SLOT(aboutActionClicked()));

	// card preview
	connect(&poolWindow_, SIGNAL(selectCardChanged(mtg::LayoutType, QStringList)), &cardWindow_, SLOT(changeCardPicture(mtg::LayoutType, QStringList)));

	// add to collection from pool
	connect(&poolWindow_, SIGNAL(addToCollection(int)), &collectionWindow_, SLOT(addToCollection(int)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::loadSettings()
{
	bool mainWindowVisibleDefault = true;
	#ifdef __APPLE__
		mainWindowVisibleDefault = false;
	#endif
	QSettings settings;
	resize(settings.value("mainwindow/size", QSize(480, 48)).toSize());
	move(settings.value("mainwindow/pos", QPoint(0, 0)).toPoint());
	setVisible(true);
	setVisible(settings.value("mainwindow/visible", mainWindowVisibleDefault).toBool());
	poolWindow_.resize(settings.value("poolwindow/size", QSize(1428, 361)).toSize());
	poolWindow_.move(settings.value("poolwindow/pos", QPoint(486, 0)).toPoint());
	poolWindow_.setVisible(settings.value("poolwindow/visible", true).toBool());
	poolWindow_.loadSettings();
	cardWindow_.resize(settings.value("cardwindow/size", QSize(480, 680)).toSize());
	cardWindow_.move(settings.value("cardwindow/pos", QPoint(0, 77)).toPoint());
	cardWindow_.setVisible(settings.value("cardwindow/visible", true).toBool());
	collectionWindow_.resize(settings.value("collectionwindow/size", QSize(1428, 367)).toSize());
	collectionWindow_.move(settings.value("collectionwindow/pos", QPoint(486, 390)).toPoint());
	collectionWindow_.setVisible(settings.value("collectionwindow/visible", true).toBool());
	deckWindow_.resize(settings.value("deckwindow/size", QSize(1914, 360)).toSize());
	deckWindow_.move(settings.value("deckwindow/pos", QPoint(0, 786)).toPoint());
	deckWindow_.setVisible(settings.value("deckwindow/visible", true).toBool());
}

void MainWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("mainwindow/visible", isVisible());
	settings.setValue("mainwindow/size", size());
	settings.setValue("mainwindow/pos", pos());
	poolWindow_.saveSettings();
	settings.setValue("poolwindow/visible", poolWindow_.isVisible());
	settings.setValue("poolwindow/size", poolWindow_.size());
	settings.setValue("poolwindow/pos", poolWindow_.pos());
	settings.setValue("cardwindow/visible", cardWindow_.isVisible());
	settings.setValue("cardwindow/size", cardWindow_.size());
	settings.setValue("cardwindow/pos", cardWindow_.pos());
	settings.setValue("collectionwindow/visible", collectionWindow_.isVisible());
	settings.setValue("collectionwindow/size", collectionWindow_.size());
	settings.setValue("collectionwindow/pos", collectionWindow_.pos());
	settings.setValue("deckwindow/visible", deckWindow_.isVisible());
	settings.setValue("deckwindow/size", deckWindow_.size());
	settings.setValue("deckwindow/pos", deckWindow_.pos());
}

bool MainWindow::toQuitOrNotToQuit(QEvent* event)
{
	int ret = QMessageBox::Yes;
	// TODO: check for unsaved stuff before asking for confirmation at exit
	// ret = QMessageBox::question(this, tr("Quit?"), tr("Are you sure you want to quit?"));
	if (ret == QMessageBox::Yes)
	{
		event->accept();
		saveSettings();
		QCoreApplication::quit();
		return true;
	}
	else
	{
		event->ignore();
		return false;
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef __APPLE__
	event->accept();
#else
	toQuitOrNotToQuit(event);
#endif
}

void MainWindow::poolWindowActionToggled(bool show)
{
	poolWindow_.setVisible(show);
}

void MainWindow::cardWindowActionToggled(bool show)
{
	cardWindow_.setVisible(show);
}

void MainWindow::collectionWindowActionToggled(bool show)
{
	collectionWindow_.setVisible(show);
}

void MainWindow::deckWindowActionToggled(bool show)
{
	deckWindow_.setVisible(show);
}

void MainWindow::optionsActionClicked()
{
	OptionsDialog options(this);
	options.exec();
	if (options.isPoolReloadRequired())
	{
		poolWindow_.reload();
	}
}

void MainWindow::aboutActionClicked()
{
	AboutDialog about(this);
	about.exec();
}
