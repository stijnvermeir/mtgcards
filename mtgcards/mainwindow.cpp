#include "mainwindow.h"

#include "optionsdialog.h"
#include "aboutdialog.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "deckmanager.h"
#include "settings.h"
#include "filtereditordialog.h"

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
	poolShowAction->setText("Show");
	ui_.menuPool->addAction(poolShowAction);
	ui_.menuPool->addSeparator();

	QAction* collectionShowAction = ui_.collectionDock->toggleViewAction();
	collectionShowAction->setText("Show");
	ui_.menuCollection->addAction(collectionShowAction);
	ui_.menuCollection->addSeparator();

	QAction* cardShowAction = ui_.cardDock->toggleViewAction();
	cardShowAction->setText("Show");
	ui_.menuCard->addAction(cardShowAction);

	poolDock_ = new PoolDock(ui_, this);
	collectionDock_ = new CollectionDock(ui_, this);
	cardDock_ = new CardDock(ui_, this);
	deckWindow_ = new DeckWindow(ui_, this);

	loadSettings();

	// options
	connect(ui_.actionOptions, SIGNAL(triggered()), this, SLOT(optionsActionClicked()));

	// about
	connect(ui_.actionAbout, SIGNAL(triggered()), this, SLOT(aboutActionClicked()));
	connect(ui_.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// import decks
	connect(ui_.actionImport_dec, SIGNAL(triggered()), this, SLOT(importDec()));

	// global filter
	connect(ui_.actionGlobalFilter, SIGNAL(triggered()), this, SLOT(globalFilter()));
	connect(this, SIGNAL(globalFilterChanged()), poolDock_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), collectionDock_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), deckWindow_, SLOT(handleGlobalFilterChanged()));

	// online manual
	connect(ui_.actionOnlineManual, SIGNAL(triggered()), this, SLOT(onlineManual()));

	// card preview
	connect(poolDock_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));
	connect(collectionDock_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));
	connect(deckWindow_, SIGNAL(selectedCardChanged(int)), cardDock_, SLOT(changeCardPicture(int)));
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
	if (false) // TODO
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

void MainWindow::importDec()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, "Import .dec", QDir::homePath(), "Decks (*.dec)");
	if (!filenames.empty())
	{
		bool yesToAll = false;
		bool noToAll = false;
		QStringList decksWithIssues;
		for (const QString& filename : filenames)
		{
			QStringList errors;
			QFile importFile(filename);
			if (importFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QFileInfo fileInfo(filename);
				QString newFilename = Settings::instance().getDecksDir() + QDir::separator() + fileInfo.baseName() + ".deck";
				if (QFileInfo(newFilename).exists())
				{
					errors << ("Deck '" + fileInfo.baseName() + "' already exists");
				}
				Deck deck;
				deck.setActive(false);
				QStringList lines;
				QTextStream in(&importFile);
				while (!in.atEnd())
				{
					auto line = in.readLine().trimmed();
					if (line.isEmpty() || line.startsWith("//"))
					{
						continue;
					}
					lines << line;
				}
				importFile.close();

				for (int i = 0; i < lines.size(); ++i)
				{
					QTextStream stream(&lines[i]);
					int amount;
					stream >> amount;
					QString set;
					stream >> set;
					set = set.remove('[').remove(']').toUpper();
					QString name = stream.readAll().trimmed();
					qDebug() << "Amount" << amount;
					qDebug() << "Set" << set;
					qDebug() << "Name" << name;
					if (name.contains("//"))
					{
						name = name.split(" // ").first();
						qDebug() << "Corrected name" << name;
					}
					int dataRowIndex = mtg::CardData::instance().findRowFast(set, name);
					if (dataRowIndex != -1)
					{
						deck.setQuantity(dataRowIndex, amount);
					}
					else
					{
						errors << (set + " " + name + " not found");
					}
				}

				bool ok = true;
				if (!errors.empty())
				{
					decksWithIssues << fileInfo.baseName();
					if (!yesToAll && !noToAll)
					{
						QMessageBox msgBox;
						msgBox.setWindowTitle("Issues");
						msgBox.setText("There were some issues importing deck <i>" + fileInfo.baseName() + "</i>. Do you want to continue to import this deck?");
						msgBox.setInformativeText("See details to see the issues.");
						msgBox.setDetailedText(errors.join("\n"));
						msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
						msgBox.setDefaultButton(QMessageBox::NoToAll);
						msgBox.setIcon(QMessageBox::Warning);
						int ret = msgBox.exec();
						if (ret == QMessageBox::No)
						{
							ok = false;
						}
						if (ret == QMessageBox::NoToAll)
						{
							noToAll = true;
						}
						if (ret == QMessageBox::YesToAll)
						{
							yesToAll = true;
						}
					}
					if (noToAll)
					{
						ok = false;
					}
				}

				if (ok)
				{
					deck.save(newFilename);
					//deckWindow_.openDeck(newFilename);
				}
			}
		}
		if (!decksWithIssues.empty())
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Decks with issues");
			msgBox.setText("There were some decks with import issues.");
			msgBox.setInformativeText("See details to see which decks had issues.");
			msgBox.setDetailedText(decksWithIssues.join("\n"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
		else
		{
			QMessageBox::information(0, "Success", "All decks were imported successfully.");
		}
	}
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

void MainWindow::onlineManual()
{
	QDesktopServices::openUrl(QUrl("https://github.com/stijnvermeir/mtgcards/blob/master/manual.md"));
}
