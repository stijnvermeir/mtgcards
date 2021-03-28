#include "mainwindow.h"

#include "optionsdialog.h"
#include "aboutdialog.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "deckmanager.h"
#include "settings.h"
#include "filtereditordialog.h"
#include "prices.h"
#include "util.h"

#include <QDebug>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QProgressDialog>
#include <QXmlStreamReader>
#include <QDesktopServices>
#include <QInputDialog>
#include <QScreen>
#include <QThread>

namespace {

 static const int LAYOUT_VERSION = 1;

void moveToCenterOfScreen(QDialog* dialog)
{
	const QRect screen = QGuiApplication::screens().first()->availableGeometry();
	dialog->move(screen.center() - dialog->rect().center());
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
    , poolDock_(nullptr)
    , collectionDock_(nullptr)
    , cardDock_(nullptr)
    , deckWindow_(nullptr)
{
	ui_.setupUi(this);

	QAction* poolShowAction = ui_.poolDock->toggleViewAction();
	poolShowAction->setText("Show Pool");
	ui_.menuWindow->addAction(poolShowAction);

	QAction* collectionShowAction = ui_.collectionDock->toggleViewAction();
	collectionShowAction->setText("Show Collection");
	ui_.menuWindow->addAction(collectionShowAction);

	QAction* cardShowAction = ui_.cardDock->toggleViewAction();
	cardShowAction->setText("Show Card Preview");
	ui_.menuWindow->addAction(cardShowAction);

	poolDock_ = new PoolDock(ui_, this);
	collectionDock_ = new CollectionDock(ui_, this);
	cardDock_ = new CardDock(ui_, this);
	deckWindow_ = new DeckWindow(ui_, this);

	loadSettings();

	// options
	connect(ui_.actionOptions, SIGNAL(triggered()), this, SLOT(optionsActionClicked()));
	connect(ui_.actionUpdatePrices, SIGNAL(triggered()), this, SLOT(updatePrices()));

	// about
	connect(ui_.actionAbout, SIGNAL(triggered()), this, SLOT(aboutActionClicked()));
	connect(ui_.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// global filter
	connect(ui_.actionGlobalFilter, SIGNAL(triggered()), this, SLOT(globalFilter()));
	connect(this, SIGNAL(globalFilterChanged()), poolDock_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), collectionDock_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), deckWindow_, SLOT(handleGlobalFilterChanged()));

	// card preview
	connect(poolDock_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));
	connect(collectionDock_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));
	connect(deckWindow_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));

	// add / remove
	connect(poolDock_, SIGNAL(addToCollection(QVector<int>)), collectionDock_, SLOT(addToCollection(QVector<int>)));
	connect(poolDock_, SIGNAL(removeFromCollection(QVector<int>)), collectionDock_, SLOT(removeFromCollection(QVector<int>)));
	connect(poolDock_, SIGNAL(addToDeck(QVector<int>)), deckWindow_, SLOT(addToDeck(QVector<int>)));
	connect(poolDock_, SIGNAL(removeFromDeck(QVector<int>)), deckWindow_, SLOT(removeFromDeck(QVector<int>)));
	connect(collectionDock_, SIGNAL(addToDeck(QVector<int>)), deckWindow_, SLOT(addToDeck(QVector<int>)));
	connect(collectionDock_, SIGNAL(removeFromDeck(QVector<int>)), deckWindow_, SLOT(removeFromDeck(QVector<int>)));
	connect(deckWindow_, SIGNAL(addToCollection(QVector<int>)), collectionDock_, SLOT(addToCollection(QVector<int>)));
	connect(deckWindow_, SIGNAL(addToCollection(QVector<QPair<int,int>>)), collectionDock_, SLOT(addToCollection(QVector<QPair<int,int>>)));
	connect(deckWindow_, SIGNAL(removeFromCollection(QVector<int>)), collectionDock_, SLOT(removeFromCollection(QVector<int>)));

	// open used decks
	connect(collectionDock_, SIGNAL(requestOpenDeck(QString)), deckWindow_, SLOT(handleOpenDeckRequest(QString)));
}

MainWindow::~MainWindow()
{
	delete poolDock_;
	delete collectionDock_;
	delete cardDock_;
	delete deckWindow_;
}

void MainWindow::loadSettings()
{
	QSettings settings;
	restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
	if (!restoreState(settings.value("mainwindow/state").toByteArray(), LAYOUT_VERSION))
	{
		splitDockWidget(ui_.poolDock, ui_.collectionDock, Qt::Orientation::Vertical);
		resizeDocks({ui_.cardDock}, {360}, Qt::Orientation::Vertical);
		resizeDocks({ui_.cardDock, ui_.poolDock, ui_.collectionDock}, {300, 1000, 1000}, Qt::Orientation::Horizontal);
	}

	poolDock_->loadSettings();
	collectionDock_->loadSettings();
	deckWindow_->loadSettings();
}

void MainWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("mainwindow/geometry", saveGeometry());
	settings.setValue("mainwindow/state", saveState(LAYOUT_VERSION));

	poolDock_->saveSettings();
	collectionDock_->saveSettings();
	deckWindow_->saveSettings();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	int ret = QMessageBox::Yes;
	if (deckWindow_->hasUnsavedChanges())
	{
		ret = QMessageBox::question(0,
		                            "Are you sure?",
		                            "There are unsaved changes. Are you sure you want to quit?",
		                            QMessageBox::Yes | QMessageBox::No,
		                            QMessageBox::No);
	}
	if (ret == QMessageBox::Yes)
	{
		saveSettings();
		event->accept();
		QMainWindow::closeEvent(event);
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::optionsActionClicked()
{
	OptionsDialog options(this);
	moveToCenterOfScreen(&options);
	connect(&options, SIGNAL(fontChanged()), poolDock_, SIGNAL(fontChanged()));
	connect(&options, SIGNAL(fontChanged()), collectionDock_, SIGNAL(fontChanged()));
	connect(&options, SIGNAL(fontChanged()), deckWindow_, SIGNAL(fontChanged()));
	options.exec();
	poolDock_->updateShortcuts();
	collectionDock_->updateShortcuts();
	deckWindow_->updateShortcuts();
	disconnect(&options, SIGNAL(fontChanged()), poolDock_, SIGNAL(fontChanged()));
	disconnect(&options, SIGNAL(fontChanged()), collectionDock_, SIGNAL(fontChanged()));
	disconnect(&options, SIGNAL(fontChanged()), deckWindow_, SIGNAL(fontChanged()));
}

void MainWindow::aboutActionClicked()
{
	AboutDialog about(this);
	moveToCenterOfScreen(&about);
	about.exec();
}

void MainWindow::globalFilter()
{
	FilterEditorDialog editor;
	editor.setWindowTitle("Global filter");
	editor.setFilterRootNode(Settings::instance().getGlobalFilter());
	editor.exec();
	Settings::instance().setGlobalFilter(editor.getFilterRootNode());
	emit globalFilterChanged();
}

void MainWindow::updatePrices()
{
	if (!Util::downloadPricesFile())
	{
		return;
	}
	QProgressDialog progress("Updating prices ...", QString(), 0, 0, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
	progress.show();
	bool success = false;
	QThread* thread = QThread::create([&]()
	{
		progress.setLabelText("Decompressing ...");
		if (Util::decompressPricesFile())
		{
			progress.setLabelText("Parsing ...");
			Prices::instance().update(Settings::instance().getPricesJsonFile());
			success = true;
			QFile::remove(Settings::instance().getPricesJsonFile());
			QFile::remove(Settings::instance().getPricesBz2File());
		}
	});
	QEventLoop loop;
	QObject::connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
	thread->start();
	loop.exec();
	delete thread;
	progress.setValue(0);
	progress.close();
	if (success)
	{
		QMessageBox::information(this, "Update prices", "Prices updated.");
	}
	else
	{
		QMessageBox::critical(this, "Update prices", "Prices update failed!");
	}
}
