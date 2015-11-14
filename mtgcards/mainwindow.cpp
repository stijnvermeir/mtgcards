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

namespace {

void moveToCenterOfScreen(QDialog* dialog)
{
	const QRect screen = QApplication::desktop()->screenGeometry();
	dialog->move(screen.center() - dialog->rect().center());
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, poolWindow_()
	, cardWindow_()
	, collectionWindow_()
	, deckWindow_()
{
	ui_.setupUi(this);

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
	connect(ui_.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// import collection
	connect(ui_.actionImportCollection, SIGNAL(triggered()), this, SLOT(importCollection()));

	// import decks
	connect(ui_.actionImportDecks, SIGNAL(triggered()), this, SLOT(importDecks()));

	// global filter
	connect(ui_.actionGlobalFilter, SIGNAL(triggered()), this, SLOT(globalFilter()));
	connect(this, SIGNAL(globalFilterChanged()), &poolWindow_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), &collectionWindow_, SLOT(handleGlobalFilterChanged()));
	connect(this, SIGNAL(globalFilterChanged()), &deckWindow_, SLOT(handleGlobalFilterChanged()));

	// online manual
	connect(ui_.actionOnlineManual, SIGNAL(triggered()), this, SLOT(onlineManual()));

	// card preview
	connect(&poolWindow_, SIGNAL(selectedCardChanged(int)), &cardWindow_, SLOT(changeCardPicture(int)));
	connect(&collectionWindow_, SIGNAL(selectedCardChanged(int)), &cardWindow_, SLOT(changeCardPicture(int)));
	connect(&deckWindow_, SIGNAL(selectedCardChanged(int)), &cardWindow_, SLOT(changeCardPicture(int)));

	// add / remove
	connect(&poolWindow_, SIGNAL(addToCollection(QVector<int>)), &collectionWindow_, SLOT(addToCollection(QVector<int>)));
	connect(&poolWindow_, SIGNAL(removeFromCollection(QVector<int>)), &collectionWindow_, SLOT(removeFromCollection(QVector<int>)));
	connect(&poolWindow_, SIGNAL(addToDeck(QVector<int>)), &deckWindow_, SLOT(addToDeck(QVector<int>)));
	connect(&poolWindow_, SIGNAL(removeFromDeck(QVector<int>)), &deckWindow_, SLOT(removeFromDeck(QVector<int>)));
	connect(&collectionWindow_, SIGNAL(addToDeck(QVector<int>)), &deckWindow_, SLOT(addToDeck(QVector<int>)));
	connect(&collectionWindow_, SIGNAL(removeFromDeck(QVector<int>)), &deckWindow_, SLOT(removeFromDeck(QVector<int>)));
	connect(&deckWindow_, SIGNAL(addToCollection(QVector<int>)), &collectionWindow_, SLOT(addToCollection(QVector<int>)));
	connect(&deckWindow_, SIGNAL(addToCollection(QVector<QPair<int,int>>)), &collectionWindow_, SLOT(addToCollection(QVector<QPair<int,int>>)));
	connect(&deckWindow_, SIGNAL(removeFromCollection(QVector<int>)), &collectionWindow_, SLOT(removeFromCollection(QVector<int>)));

	// open used decks
	connect(&collectionWindow_, SIGNAL(requestOpenDeck(QString)), &deckWindow_, SLOT(handleOpenDeckRequest(QString)));
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
	collectionWindow_.loadSettings();

	deckWindow_.resize(settings.value("deckwindow/size", QSize(1914, 360)).toSize());
	deckWindow_.move(settings.value("deckwindow/pos", QPoint(0, 786)).toPoint());
	deckWindow_.setVisible(settings.value("deckwindow/visible", true).toBool());
	deckWindow_.loadSettings();
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

	collectionWindow_.saveSettings();
	settings.setValue("collectionwindow/visible", collectionWindow_.isVisible());
	settings.setValue("collectionwindow/size", collectionWindow_.size());
	settings.setValue("collectionwindow/pos", collectionWindow_.pos());

	deckWindow_.saveSettings();
	settings.setValue("deckwindow/visible", deckWindow_.isVisible());
	settings.setValue("deckwindow/size", deckWindow_.size());
	settings.setValue("deckwindow/pos", deckWindow_.pos());
}

bool MainWindow::toQuitOrNotToQuit(QEvent* event)
{
	int ret = QMessageBox::Yes;
	if (deckWindow_.hasUnsavedChanges())
	{
		ret = QMessageBox::question(0,
									"Are you sure?",
									"There are unsaved changes. Are you sure you want to quit?",
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::No);
	}
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
	moveToCenterOfScreen(&options);
	connect(&options, SIGNAL(fontChanged()), &poolWindow_, SIGNAL(fontChanged()));
	connect(&options, SIGNAL(fontChanged()), &collectionWindow_, SIGNAL(fontChanged()));
	connect(&options, SIGNAL(fontChanged()), &deckWindow_, SIGNAL(fontChanged()));
	options.exec();
	if (options.isPoolReloadRequired())
	{
		poolWindow_.reload();
		collectionWindow_.reload();
		deckWindow_.reload();
	}
	poolWindow_.updateShortcuts();
	collectionWindow_.updateShortcuts();
	deckWindow_.updateShortcuts();
	disconnect(&options, SIGNAL(fontChanged()), &poolWindow_, SIGNAL(fontChanged()));
	disconnect(&options, SIGNAL(fontChanged()), &collectionWindow_, SIGNAL(fontChanged()));
	disconnect(&options, SIGNAL(fontChanged()), &deckWindow_, SIGNAL(fontChanged()));
}

void MainWindow::aboutActionClicked()
{
	AboutDialog about(this);
	moveToCenterOfScreen(&about);
	about.exec();
}

void MainWindow::importCollection()
{
	QString filename = QFileDialog::getOpenFileName(0, "Import collection from csv", QDir::homePath(), "CSV (*.csv)");
	if (!filename.isNull())
	{
		QFile importFile(filename);
		if (importFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QStringList lines;
			QTextStream in(&importFile);
			while (!in.atEnd())
			{
				lines << in.readLine();
			}
			importFile.close();

			QSqlDatabase db = mtg::Collection::instance().getConnection();
			db.transaction();

			QProgressDialog progress("Importing ...", "Cancel", 0, lines.size());
			progress.setWindowModality(Qt::WindowModal);
			QStringList errors;
			for (int i = 0; i < lines.size(); ++i)
			{
				QStringList splitLine = lines[i].split(";");
				if (splitLine.size() >= 3)
				{
					QString set = splitLine[0];
					QString name = splitLine[1];
					if (name.contains("/"))
					{
						name = name.split("/").first();
					}
					int qty = splitLine[2].toInt();
					int dataRowIndex = mtg::CardData::instance().findRowFast(set, name);
					if (dataRowIndex != -1)
					{
						int current = mtg::Collection::instance().getQuantity(dataRowIndex);
						mtg::Collection::instance().setQuantity(dataRowIndex, current + qty);
					}
					else
					{
						errors << (set + " " + name + " not found");
					}
				}
				else
				{
					errors << ("Line " + QString::number(i+1) + " is invalid");
				}
				progress.setValue(i);
				if (progress.wasCanceled())
				{
					db.rollback();
					break;
				}
			}
			progress.setValue(lines.size());
			if (!progress.wasCanceled())
			{
				if (errors.empty())
				{
					db.commit();
				}
				else
				{
					QMessageBox msgBox;
					msgBox.setWindowTitle("Issues");
					msgBox.setText("Some cards could not be imported. Do you want to continue with the import?");
					msgBox.setInformativeText("See details to know which cards could not be imported.");
					msgBox.setDetailedText(errors.join("\n"));
					msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
					msgBox.setDefaultButton(QMessageBox::Yes);
					msgBox.setIcon(QMessageBox::Warning);
					int ret = msgBox.exec();
					if (ret == QMessageBox::Yes)
					{
						db.commit();
					}
					else
					{
						db.rollback();
					}
				}
			}
			collectionWindow_.reload();
		}
	}
}

void MainWindow::importDecks()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, "Import decks from xml", QDir::homePath(), "Decks (*.deck)");
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
				QXmlStreamReader xml(&importFile);
				while (!xml.atEnd())
				{
					xml.readNext();
					if (xml.isStartElement() && xml.name().compare("card") == 0)
					{
						QString set = xml.attributes().value("edition").toString();
						int qty = xml.attributes().value("deck").toInt();
						int sb = xml.attributes().value("sb").toInt();
						QString name = xml.readElementText();
						if (name.contains("/"))
						{
							name = name.split("/").first();
						}
						int dataRowIndex = mtg::CardData::instance().findRowFast(set, name);
						if (dataRowIndex != -1)
						{
							deck.setQuantity(dataRowIndex, qty);
							deck.setSideboard(dataRowIndex, sb);
						}
						else
						{
							errors << (set + " " + name + " not found");
						}
					}
					if (xml.hasError())
					{
						errors << xml.errorString();
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
					deckWindow_.openDeck(newFilename);
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
