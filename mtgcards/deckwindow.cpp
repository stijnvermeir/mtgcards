#include "deckwindow.h"

#include "deckwidget.h"
#include "deck.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "filtereditordialog.h"
#include "settings.h"
#include "util.h"
#include "deckstatisticsdialog.h"

#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QPdfWriter>
#include <QPainter>
#include <QLabel>
#include <QClipboard>
#include <QDebug>

DeckWindow::DeckWindow(Ui::MainWindow& ui, QWidget* parent)
    : QObject(parent)
    , ui_(ui)
	, headerState_()
	, rootFilterNode_()
    , actionNewDeck_(nullptr)
    , actionOpenDeck_(nullptr)
    , actionSaveDeck_(nullptr)
    , actionSaveDeckAs_(nullptr)
    , actionImportDec_(nullptr)
    , actionToggleDeckActive_(nullptr)
    , actionAddDeckToCollection_(nullptr)
    , actionCreateProxies_(nullptr)
    , actionStats_(nullptr)
    , commonActions_(this)
    , toolBar_(new QToolBar())
{
	actionNewDeck_ = new QAction(QIcon(":/resources/icons/blank33.svg"), "New deck", this);
	actionOpenDeck_ = new QAction(QIcon(":/resources/icons/folder215.svg"), "Open deck ...", this);
	actionSaveDeck_ = new QAction(QIcon(":/resources/icons/save20.svg"), "Save deck", this);
	actionSaveDeckAs_ = new QAction(QIcon(":/resources/icons/disc36.svg"), "Save deck as ...", this);
	actionImportDec_ = new QAction(QIcon(":/resources/icons/import.svg"), "Import deck from .dec file ...", this);
	QIcon deckActiveIcon(":/resources/icons/switches3.svg");
	deckActiveIcon.addFile(":/resources/icons/switch14.svg", QSize(), QIcon::Normal, QIcon::On);
	actionToggleDeckActive_ = new QAction(deckActiveIcon, "Toggle deck active", this);
	actionToggleDeckActive_->setCheckable(true);
	actionAddDeckToCollection_ = new QAction(QIcon(":/resources/icons/document24.svg"), "Add deck to collection", this);
	actionCreateProxies_= new QAction(QIcon(":/resources/icons/printer11.svg"), "Create proxies", this);
	actionStats_ = new QAction(QIcon(":/resources/icons/chart59.svg"), "Stats", this);
	actionCopyDeckStatsClipboard_ = new QAction(QIcon(":/resources/icons/export.svg"), "Copy deckstats.net format to clipboard", this);
	actionCopyDeckStatsClipboard_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::DeckstatsClipboard]);

	toolBar_->setIconSize(QSize(16, 16));
	toolBar_->addAction(actionNewDeck_);
	toolBar_->addAction(actionImportDec_);
	toolBar_->addAction(actionOpenDeck_);
	toolBar_->addAction(actionSaveDeck_);
	toolBar_->addAction(actionSaveDeckAs_);
	toolBar_->addAction(actionCopyDeckStatsClipboard_);
	toolBar_->addSeparator();
	toolBar_->addAction(actionToggleDeckActive_);
	toolBar_->addAction(actionStats_);
	toolBar_->addAction(actionAddDeckToCollection_);
	toolBar_->addAction(actionCreateProxies_);
	ui_.toolbarLayout->addWidget(toolBar_);

	connect(ui_.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeDeck(int)));
	connect(ui_.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChangedSlot(int)));

	connect(actionNewDeck_, SIGNAL(triggered()), this, SLOT(actionNewDeck()));
	connect(actionOpenDeck_, SIGNAL(triggered()), this, SLOT(actionOpenDeck()));
	connect(actionSaveDeck_, SIGNAL(triggered()), this, SLOT(actionSaveDeck()));
	connect(actionSaveDeckAs_, SIGNAL(triggered()), this, SLOT(actionSaveDeckAs()));
	connect(actionImportDec_, SIGNAL(triggered()), this, SLOT(actionImportDec()));
	connect(actionToggleDeckActive_, SIGNAL(triggered(bool)), this, SLOT(actionToggleDeckActive(bool)));
	connect(actionAddDeckToCollection_, SIGNAL(triggered()), this, SLOT(actionAddDeckToCollection()));
	connect(actionCreateProxies_, SIGNAL(triggered()), this, SLOT(createProxies()));
	connect(actionStats_, SIGNAL(triggered()), this, SLOT(showStatistics()));
	connect(actionCopyDeckStatsClipboard_, SIGNAL(triggered()), this, SLOT(copyDeckstatsClipboard()));

	ui_.menuFile->addAction(actionNewDeck_);
	ui_.menuFile->addAction(actionImportDec_);
	ui_.menuFile->addAction(actionOpenDeck_);
	ui_.menuFile->addAction(actionSaveDeck_);
	ui_.menuFile->addAction(actionSaveDeckAs_);
	ui_.menuFile->addAction(actionCopyDeckStatsClipboard_);

	commonActions_.connectSignals(this);
	commonActions_.addToWidget(ui_.tabWidget);
	ui_.tabWidget->addAction(actionCopyDeckStatsClipboard_);

	ui_.deckStatusBar->setViewChangerEnabled(false);
}

DeckWindow::~DeckWindow()
{
	delete toolBar_;
}

void DeckWindow::updateShortcuts()
{
	actionNewDeck_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::NewFile]);
	actionImportDec_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::ImportFile]);
	actionOpenDeck_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::OpenFile]);
	actionSaveDeck_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::SaveFile]);
	actionSaveDeckAs_->setShortcut(Settings::instance().getShortcuts()[ShortcutType::SaveFileAs]);
	commonActions_.updateShortcuts();
}

void DeckWindow::loadSettings()
{
	QSettings settings;
	if (settings.contains("deckwindow/headerstate"))
	{
		headerState_ = settings.value("deckwindow/headerstate").toString();
	}
	if (settings.contains("deckwindow/filterEnable"))
	{
		commonActions_.getEnableFilter()->setChecked(settings.value("deckwindow/filterEnable").toBool());
	}
	if (settings.contains("deckwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("deckwindow/filter").toString().toUtf8()));
	}
	if (settings.contains("deckwindow/openfiles"))
	{
		QStringList openFiles = settings.value("deckwindow/openfiles").toStringList();
		for (const auto& openFile : openFiles)
		{
			createDeckWidget(openFile);
		}
	}
	updateStatusBar();
	updateShortcuts();
}

void DeckWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("deckwindow/filterEnable", commonActions_.getEnableFilter()->isChecked());
	if (rootFilterNode_)
	{
		settings.setValue("deckwindow/filter", QString(rootFilterNode_->toJson().toJson(QJsonDocument::Compact)));
	}
	else
	{
		settings.remove("deckwindow/filter");
	}
	if (!headerState_.isEmpty())
	{
		settings.setValue("deckwindow/headerstate", headerState_);
	}
	QStringList openFiles;
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->widget(tabIndex));
		if (deckWidget && !deckWidget->deck().getFilename().isEmpty())
		{
			openFiles.append(deckWidget->deck().getFilename());
		}
	}
	settings.setValue("deckwindow/openfiles", openFiles);
}

bool DeckWindow::hasUnsavedChanges() const
{
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		QWidget* widget = ui_.tabWidget->widget(tabIndex);
		if (widget)
		{
			if (static_cast<DeckWidget*>(widget)->deck().hasUnsavedChanges())
			{
				return true;
			}
		}
	}
	return false;
}

void DeckWindow::openDeck(const QString& deckId)
{
	handleOpenDeckRequest(deckId);
}

void DeckWindow::updateStatusBar()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		const auto& model = deckWidget->model();
		auto getValue = [&model](int row, const mtg::ColumnType& columnType)
		{
			int column = model.columnToIndex(columnType);
			QModelIndex index = model.index(row, column);
			return model.data(index);
		};
		int numLands = 0;
		int numCreatures = 0;
		int numCopies = 0;
		double sumPrice = 0.0;
		for (int row = 0; row < model.rowCount(); ++row)
		{
			int quantity = getValue(row, mtg::ColumnType::Quantity).toInt();
			QString type = getValue(row, mtg::ColumnType::Type).toString();
			if (type.contains("Creature"))
			{
				numCreatures += quantity;
			}
			if (type.contains("Land"))
			{
				numLands += quantity;
			}
			numCopies += quantity;

			sumPrice += quantity * getValue(row, mtg::ColumnType::Price).toDouble();
		}
		QString message;
		QTextStream str(&message);
		str << "Showing " << model.rowCount() << " of " << deckWidget->deck().getNumRows() << " cards";
		str << " (" << numCopies << " copies, " << numLands << " lands, " << numCreatures << " creatures, " << numCopies - numCreatures - numLands << " others)";
		if (!deckWidget->isColumnHidden(mtg::ColumnType::Price))
		{
			str << " [Sum price: " << sumPrice << "]";
		}
		ui_.deckStatusBar->setMessage(message);
		actionToggleDeckActive_->setChecked(deckWidget->deck().isActive());
	}
	else
	{
		ui_.deckStatusBar->setMessage("");
		actionToggleDeckActive_->setChecked(false);
	}
}

DeckWidget* DeckWindow::createDeckWidget(const QString& filename)
{
	if (!filename.isEmpty() && !QFileInfo(filename).exists())
	{
		return nullptr;
	}

	// prevent opening deck twice
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->widget(tabIndex));
		if (deckWidget->deck().getFilename() == filename)
		{
			ui_.tabWidget->setCurrentWidget(deckWidget);
			return nullptr;
		}
	}

	DeckWidget* deckWidget = new DeckWidget(filename, commonActions_);
	deckWidget->setHeaderState(headerState_);
	if (commonActions_.getEnableFilter()->isChecked())
	{
		deckWidget->setFilterRootNode(rootFilterNode_);
	}
	connect(deckWidget, SIGNAL(selectedCardChanged(int)), this, SIGNAL(selectedCardChanged(int)));
	connect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
	connect(deckWidget, SIGNAL(deckEdited()), this, SLOT(deckEdited()));
	connect(deckWidget, SIGNAL(searchStringChanged(QString)), ui_.deckStatusBar, SLOT(setSearch(QString)));
	connect(this, SIGNAL(fontChanged()), deckWidget, SIGNAL(fontChanged()));
	ui_.tabWidget->addTab(deckWidget, deckWidget->deck().getDisplayName());
	ui_.tabWidget->setCurrentWidget(deckWidget);
	return deckWidget;
}

void DeckWindow::destroyDeckWidget(DeckWidget* deckWidget)
{
	if (deckWidget)
	{
		int index = ui_.tabWidget->indexOf(deckWidget);
		if (index != -1)
		{
			if (deckWidget->deck().hasUnsavedChanges())
			{
				int ret = QMessageBox::question(ui_.tabWidget,
												"Save before close?",
												"This deck has unsaved changes. Do you want to save your changes?",
												QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
												QMessageBox::Cancel);
				if (ret == QMessageBox::Cancel)
				{
					return;
				}
				if (ret == QMessageBox::Save)
				{
					saveDeck(deckWidget, false);
				}
			}
			disconnect(deckWidget, SIGNAL(selectedCardChanged(int)), this, SIGNAL(selectedCardChanged(int)));
			disconnect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
			disconnect(deckWidget, SIGNAL(deckEdited()), this, SLOT(deckEdited()));
			disconnect(deckWidget, SIGNAL(searchStringChanged(QString)), ui_.deckStatusBar, SLOT(setSearch(QString)));
			disconnect(this, SIGNAL(fontChanged()), deckWidget, SIGNAL(fontChanged()));
			deckWidget->close();
			ui_.tabWidget->removeTab(index);
			delete deckWidget;
		}
	}
}

void DeckWindow::saveDeck(DeckWidget* deckWidget, bool saveAs)
{
	if (deckWidget)
	{
		QString filename = deckWidget->deck().getFilename();
		if (filename.isEmpty() || saveAs)
		{
			filename = QFileDialog::getSaveFileName(ui_.tabWidget , "Save Deck file", Settings::instance().getDecksDir(), "Decks (*.deck)");
		}
		if (!filename.isEmpty())
		{
			deckWidget->save(filename);
			ui_.tabWidget->setTabText(ui_.tabWidget->indexOf(deckWidget), deckWidget->deck().getDisplayName());
		}
	}
}

void DeckWindow::addToDeck(const QVector<int>& dataRowIndices)
{
	if (ui_.tabWidget->currentWidget())
	{
		static_cast<DeckWidget*>(ui_.tabWidget->currentWidget())->addToDeck(dataRowIndices);
	}
}

void DeckWindow::removeFromDeck(const QVector<int>& dataRowIndices)
{
	if (ui_.tabWidget->currentWidget())
	{
		static_cast<DeckWidget*>(ui_.tabWidget->currentWidget())->removeFromDeck(dataRowIndices);
	}
}

void DeckWindow::selectedCardChangedSlot()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		emit selectedCardChanged(static_cast<DeckWidget*>(widget)->currentDataRowIndex());
	}
}

void DeckWindow::closeDeck(int index)
{
	destroyDeckWidget(static_cast<DeckWidget*>(ui_.tabWidget->widget(index)));
}

void DeckWindow::currentTabChangedSlot(int)
{
	selectedCardChangedSlot();
	updateStatusBar();
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->resetSearchString();
	}
	else
	{
		ui_.deckStatusBar->setMessage("");
	}
}

void DeckWindow::actionNewDeck()
{
	createDeckWidget();
}

void DeckWindow::actionOpenDeck()
{
	auto filename = QFileDialog::getOpenFileName(ui_.tabWidget, "Open Deck file", Settings::instance().getDecksDir(), "Decks (*.deck)");
	if (!filename.isNull())
	{
		createDeckWidget(filename);
	}
}

void DeckWindow::actionSaveDeck()
{
	saveDeck(static_cast<DeckWidget*>(ui_.tabWidget->currentWidget()), false);
}

void DeckWindow::actionSaveDeckAs()
{
	saveDeck(static_cast<DeckWidget*>(ui_.tabWidget->currentWidget()), true);
}

void DeckWindow::actionImportDec()
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
					auto line = lines[i];
					if (line.contains('#'))
					{
						line = line.split('#').first();
					}
					bool sideBoard = false;
					if (line.startsWith("SB: "))
					{
						line.remove("SB: ");
						sideBoard = true;
					}
					bool withSet = false;
					if (line.contains('['))
					{
						withSet = true;
					}
					QTextStream stream(&line);
					int amount;
					stream >> amount;
					QString set;
					if (withSet)
					{
						stream >> set;
						set = set.remove('[').remove(']').toUpper();
					}
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
						if (sideBoard)
						{
							deck.setSideboard(dataRowIndex, amount);
						}
						else
						{
							deck.setQuantity(dataRowIndex, amount);
						}
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
					openDeck(newFilename);
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

void DeckWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	if (commonActions_.getEnableFilter()->isChecked())
	{
		for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
		{
			QWidget* widget = ui_.tabWidget->widget(tabIndex);
			if (widget)
			{
				if (commonActions_.getEnableFilter()->isChecked())
				{
					static_cast<DeckWidget*>(widget)->setFilterRootNode(rootFilterNode_);
				}
			}
		}
	}
	updateStatusBar();
}

void DeckWindow::actionEnableFilter(bool enable)
{
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		QWidget* widget = ui_.tabWidget->widget(tabIndex);
		if (widget)
		{
			if (enable)
			{
				static_cast<DeckWidget*>(widget)->setFilterRootNode(rootFilterNode_);
			}
			else
			{
				static_cast<DeckWidget*>(widget)->setFilterRootNode(FilterNode::Ptr());
			}
		}
	}
	updateStatusBar();
}

void DeckWindow::actionAddToCollection()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		emit addToCollection(static_cast<DeckWidget*>(widget)->currentDataRowIndices());
	}
}

void DeckWindow::actionAddDeckToCollection()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		auto response = QMessageBox::question(widget, "Are you sure?", "Are you sure you want to add the entire deck to your collection?", QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
		if (response == QMessageBox::Yes)
		{
			emit addToCollection(static_cast<DeckWidget*>(widget)->deck().getQuantities());
		}
	}
}

void DeckWindow::actionRemoveFromCollection()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		emit removeFromCollection(static_cast<DeckWidget*>(widget)->currentDataRowIndices());
	}
}

void DeckWindow::actionAddToDeck()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		DeckWidget* deckWidget = static_cast<DeckWidget*>(widget);
		deckWidget->addToDeck(deckWidget->currentDataRowIndices());
	}
}

void DeckWindow::actionRemoveFromDeck()
{
	QWidget* widget = ui_.tabWidget->currentWidget();
	if (widget)
	{
		DeckWidget* deckWidget = static_cast<DeckWidget*>(widget);
		deckWidget->removeFromDeck(deckWidget->currentDataRowIndices());
	}
}

void DeckWindow::actionToggleDeckActive(bool active)
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->setDeckActive(active);
	}
}

void DeckWindow::createProxies()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		auto indices = deckWidget->currentDataRowIndices();
		if (indices.isEmpty())
		{
			QMessageBox::information(ui_.tabWidget, "No selection", "No cards were selected. Please select the cards you want to make proxies for.");
			return;
		}

		auto fullBorders = QMessageBox::question(ui_.tabWidget, "Borders cropped?", "Would you like to use cropped or full borders?", "Cropped", "Full");

		auto pdfFile = QFileDialog::getSaveFileName(ui_.tabWidget, "Save to Pdf", QDir::homePath(), "Pdf (*.pdf)");
		if (pdfFile.isNull())
			return;

		QPdfWriter pdf(pdfFile);
		pdf.setPageSize(QPageSize(QPageSize::A4));
		QPainter painter(&pdf);

		int printIndex = 0;
		auto printCardLambda = [&printIndex, &painter, &pdf](const QString& filename)
		{
			const QSizeF CARD_SIZE(pdf.logicalDpiX() * 2.49, pdf.logicalDpiY() * 3.48);
			QPointF topLeft((printIndex % 3) * CARD_SIZE.width(), (printIndex / 3) * CARD_SIZE.height());
			painter.drawImage(QRectF(topLeft, CARD_SIZE), QImage(filename));
			++printIndex;
			if (printIndex >= 9)
			{
				pdf.newPage();
				printIndex = 0;
			}
		};

		QStringList missing;
		for (int index : indices)
		{
			auto quantity = deckWidget->deck().getQuantity(index);
			auto pictureInfo = mtg::CardData::instance().getPictureInfo(index, fullBorders, true);
			if (pictureInfo.missing.empty())
			{
				for (int i = 0; i < quantity; ++i)
				{
					printCardLambda(pictureInfo.filenames.first());
					if (pictureInfo.layout == mtg::LayoutType::Transform || pictureInfo.layout == mtg::LayoutType::ModalDFC)
					{
						printCardLambda(pictureInfo.filenames.last());
					}
				}
			}
			else
			{
				missing << pictureInfo.missing;
			}
		}

		painter.end();

		if (missing.empty())
		{
			QMessageBox::information(ui_.tabWidget, "Success", "Proxy generation successful.");
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Issues");
			msgBox.setText("Some card images could not be found and were skipped during proxy generation.");
			msgBox.setInformativeText("See details to know which files are missing.");
			msgBox.setDetailedText(missing.join("\n"));
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
	}
}

void DeckWindow::showStatistics()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		DeckStatisticsDialog dlg(deckWidget->deck(), ui_.tabWidget);
		dlg.exec();
	}
}

void DeckWindow::copyDeckstatsClipboard()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		qDebug() << "Copying deck onto clipboard in deckstats.net format.";
		QString text;
		for (int i = 0; i < deckWidget->deck().getNumRows(); ++i)
		{
			text += deckWidget->deck().get(i, mtg::ColumnType::Quantity).toString();
			text += " [";
			text += deckWidget->deck().get(i, mtg::ColumnType::SetCode).toString();
			text += "] ";
			text += deckWidget->deck().get(i, mtg::ColumnType::Name).toString();
			text += "\n";
		}
		QApplication::clipboard()->setText(text);
	}
}

void DeckWindow::actionDownloadCardArt()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->downloadCardArt();
	}
}

void DeckWindow::actionFetchOnlineData()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->fetchOnlineData();
	}
}

void DeckWindow::deckEdited()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget && deckWidget->deck().hasUnsavedChanges())
	{
		QString tabText = deckWidget->deck().getDisplayName() + "*";
		ui_.tabWidget->setTabText(ui_.tabWidget->indexOf(deckWidget), tabText);
	}
	updateStatusBar();
}

void DeckWindow::handleOpenDeckRequest(const QString& deckId)
{
	createDeckWidget(deckId);
}

void DeckWindow::headerStateChangedSlot(const QString& headerState)
{
	headerState_ = headerState;
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->widget(tabIndex));
		if (deckWidget && deckWidget != ui_.tabWidget->currentWidget())
		{
			disconnect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
			deckWidget->setHeaderState(headerState_);
			connect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
		}
	}
	updateStatusBar();
}

void DeckWindow::handleGlobalFilterChanged()
{
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		QWidget* widget = ui_.tabWidget->widget(tabIndex);
		if (widget)
		{
			if (commonActions_.getEnableFilter()->isChecked())
			{
				static_cast<DeckWidget*>(widget)->setFilterRootNode(rootFilterNode_);
			}
			else
			{
				static_cast<DeckWidget*>(widget)->setFilterRootNode(FilterNode::Ptr());
			}
		}
	}
	updateStatusBar();
}
