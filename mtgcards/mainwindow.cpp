#include "mainwindow.h"

#include "optionsdialog.h"
#include "enum.h"

#include <QDebug>
#include <QSettings>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolWindow_(this)
	, cardWindow_(this)
	, collectionWindow_(this)
	, deckWindow_(this)
{
	ui_.setupUi(this);

	qRegisterMetaType<layout_type_t>();

	QSettings settings;
	resize(settings.value("mainwindow/size", QSize(480, 48)).toSize());
	move(settings.value("mainwindow/pos", QPoint(0, 0)).toPoint());

	poolWindow_.setVisible(settings.value("poolwindow/visible", true).toBool());
	ui_.actionPoolWindow->setChecked(poolWindow_.isVisible());
	connect(ui_.actionPoolWindow, SIGNAL(toggled(bool)), this, SLOT(poolWindowActionToggled(bool)));
	connect(&poolWindow_, SIGNAL(windowClosed(bool)), ui_.actionPoolWindow, SLOT(setChecked(bool)));

	cardWindow_.setVisible(settings.value("cardwindow/visible", true).toBool());
	ui_.actionCardWindow->setChecked(cardWindow_.isVisible());
	connect(ui_.actionCardWindow, SIGNAL(toggled(bool)), this, SLOT(cardWindowActionToggled(bool)));
	connect(&cardWindow_, SIGNAL(windowClosed(bool)), ui_.actionCardWindow, SLOT(setChecked(bool)));

	collectionWindow_.setVisible(settings.value("collectionwindow/visible", true).toBool());
	ui_.actionCollectionWindow->setChecked(collectionWindow_.isVisible());
	connect(ui_.actionCollectionWindow, SIGNAL(toggled(bool)), this, SLOT(collectionWindowActionToggled(bool)));
	connect(&collectionWindow_, SIGNAL(windowClosed(bool)), ui_.actionCollectionWindow, SLOT(setChecked(bool)));

	deckWindow_.setVisible(settings.value("deckwindow/visible", true).toBool());
	ui_.actionDeckWindow->setChecked(deckWindow_.isVisible());
	connect(ui_.actionDeckWindow, SIGNAL(toggled(bool)), this, SLOT(deckWindowActionToggled(bool)));
	connect(&deckWindow_, SIGNAL(windowClosed(bool)), ui_.actionDeckWindow, SLOT(setChecked(bool)));

	connect(ui_.actionOptions, SIGNAL(triggered()), this, SLOT(optionsActionClicked()));

	connect(&poolWindow_, SIGNAL(selectCardChanged(layout_type_t, QStringList)), &cardWindow_, SLOT(changeCardPicture(layout_type_t, QStringList)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent* event)
{
	qDebug() << "Closing Main Window";

	QSettings settings;
	settings.setValue("poolwindow/visible", poolWindow_.isVisible());
	settings.setValue("cardwindow/visible", cardWindow_.isVisible());
	settings.setValue("collectionwindow/visible", collectionWindow_.isVisible());
	settings.setValue("deckwindow/visible", deckWindow_.isVisible());

	if (poolWindow_.isVisible())
	{
		poolWindow_.close();
	}
	if (cardWindow_.isVisible())
	{
		cardWindow_.close();
	}
	if (collectionWindow_.isVisible())
	{
		collectionWindow_.close();
	}
	if (deckWindow_.isVisible())
	{
		deckWindow_.close();
	}

	settings.setValue("mainwindow/size", size());
	settings.setValue("mainwindow/pos", pos());

	if (event)
	{
		event->accept();
	}
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
	qDebug() << "Creating Options Dialog";
	OptionsDialog options(this);
	qDebug() << "Showing Options Dialog";
	options.exec();
	qDebug() << "Options Dialog closed";
	if (options.isPoolReloadRequired())
	{
		poolWindow_.reload();
	}
}
