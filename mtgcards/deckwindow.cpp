#include "deckwindow.h"

#include "deckwidget.h"
#include "deck.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "filtereditordialog.h"
#include "settings.h"
#include "util.h"
#include "deckstatisticsdialog.h"

#include <mkm/exception.h>

#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QPdfWriter>
#include <QPainter>
#include <QLabel>
#include <QDebug>

namespace {

const QString DEFAULT_HEADER_STATE = "{\"sections\": [{\"hidden\": true,\"size\": 100,\"visualIndex\": 0},{\"hidden\": false,\"size\": 46,\"visualIndex\": 2},{\"hidden\": true,\"size\": 100,\"visualIndex\": 8},{\"hidden\": true,\"size\": 133,\"visualIndex\": 9},{\"hidden\": true,\"size\": 100,\"visualIndex\": 3},{\"hidden\": true,\"size\": 377,\"visualIndex\": 1},{\"hidden\": true,\"size\": 100,\"visualIndex\": 10},{\"hidden\": true,\"size\": 40,\"visualIndex\": 11},{\"hidden\": true,\"size\": 36,\"visualIndex\": 12},{\"hidden\": false,\"size\": 305,\"visualIndex\": 4},{\"hidden\": true,\"size\": 114,\"visualIndex\": 5},{\"hidden\": false,\"size\": 37,\"visualIndex\": 6},{\"hidden\": false,\"size\": 29,\"visualIndex\": 7},{\"hidden\": false,\"size\": 222,\"visualIndex\": 14},{\"hidden\": false,\"size\": 34,\"visualIndex\": 15},{\"hidden\": false,\"size\": 124,\"visualIndex\": 16},{\"hidden\": false,\"size\": 266,\"visualIndex\": 17},{\"hidden\": true,\"size\": 100,\"visualIndex\": 18},{\"hidden\": true,\"size\": 100,\"visualIndex\": 19},{\"hidden\": true,\"size\": 100,\"visualIndex\": 20},{\"hidden\": false,\"size\": 90,\"visualIndex\": 21},{\"hidden\": false,\"size\": 188,\"visualIndex\": 22},{\"hidden\": true,\"size\": 100,\"visualIndex\": 23},{\"hidden\": true,\"size\": 100,\"visualIndex\": 26},{\"hidden\": false,\"size\": 24,\"visualIndex\": 24},{\"hidden\": false,\"size\": 26,\"visualIndex\": 25},{\"hidden\": true,\"size\": 100,\"visualIndex\": 27},{\"hidden\": true,\"size\": 127,\"visualIndex\": 28},{\"hidden\": true,\"size\": 100,\"visualIndex\": 29},{\"hidden\": false,\"size\": 62,\"visualIndex\": 13},{\"hidden\": false,\"size\": 110,\"visualIndex\": 30},{\"hidden\": true,\"size\": 113,\"visualIndex\": 31},{\"hidden\": true,\"size\": 87,\"visualIndex\": 32},{\"hidden\": false,\"size\": 85,\"visualIndex\": 33},{\"hidden\": true,\"size\": 100,\"visualIndex\": 34}],\"sortIndicatorOrder\": 0,\"sortIndicatorSection\": 16}";

}

DeckWindow::DeckWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui_()
	, headerState_()
	, rootFilterNode_()
	, permanentStatusBarLabel_(new QLabel())
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
	ui_.setupUi(this);

	connect(ui_.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeDeck(int)));
	connect(ui_.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChangedSlot(int)));
	connect(ui_.actionNewDeck, SIGNAL(triggered()), this, SLOT(actionNewDeck()));
	connect(ui_.actionOpenDeck, SIGNAL(triggered()), this, SLOT(actionOpenDeck()));
	connect(ui_.actionSaveDeck, SIGNAL(triggered()), this, SLOT(actionSaveDeck()));
	connect(ui_.actionSaveDeckAs, SIGNAL(triggered()), this, SLOT(actionSaveDeckAs()));
	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
	connect(ui_.actionEnableFilter, SIGNAL(triggered(bool)), this, SLOT(actionEnableFilter(bool)));
	connect(ui_.actionAddToCollection, SIGNAL(triggered()), this, SLOT(actionAddToCollection()));
	connect(ui_.actionRemoveFromCollection, SIGNAL(triggered()), this, SLOT(actionRemoveFromCollection()));
	connect(ui_.actionAddToDeck, SIGNAL(triggered()), this, SLOT(actionAddToDeck()));
	connect(ui_.actionRemoveFromDeck, SIGNAL(triggered()), this, SLOT(actionRemoveFromDeck()));
	connect(ui_.actionAddDeckToCollection, SIGNAL(triggered()), this, SLOT(actionAddDeckToCollection()));
	connect(ui_.actionToggleDeckActive, SIGNAL(triggered(bool)), this, SLOT(actionToggleDeckActive(bool)));
	connect(ui_.actionCreateProxies, SIGNAL(triggered()), this, SLOT(createProxies()));
	connect(ui_.actionStats, SIGNAL(triggered()), this, SLOT(showStatistics()));
	connect(ui_.actionDownloadCardArt, SIGNAL(triggered()), this, SLOT(downloadCardArt()));
	connect(ui_.actionFetchOnlineData, SIGNAL(triggered()), this, SLOT(fetchOnlineData()));
	connect(ui_.actionAddToWantslist, SIGNAL(triggered()), this, SLOT(addToWantslist()));

	ui_.statusBar->addPermanentWidget(new QLabel("Search: "));
	ui_.statusBar->addPermanentWidget(permanentStatusBarLabel_);
}

DeckWindow::~DeckWindow()
{
}

void DeckWindow::updateShortcuts()
{
	ui_.actionNewDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::NewFile]);
	ui_.actionOpenDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::OpenFile]);
	ui_.actionSaveDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::SaveFile]);
	ui_.actionSaveDeckAs->setShortcut(Settings::instance().getShortcuts()[ShortcutType::SaveFileAs]);
	ui_.actionAdvancedFilter->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AdvancedFilter]);
	ui_.actionAddToCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToCollection]);
	ui_.actionRemoveFromCollection->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromCollection]);
	ui_.actionAddToDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::AddToDeck]);
	ui_.actionRemoveFromDeck->setShortcut(Settings::instance().getShortcuts()[ShortcutType::RemoveFromDeck]);
}

void DeckWindow::loadSettings()
{
	QSettings settings;
	headerState_ = settings.value("deckwindow/headerstate", DEFAULT_HEADER_STATE).toString();
	if (settings.contains("deckwindow/filterEnable"))
	{
		ui_.actionEnableFilter->setChecked(settings.value("deckwindow/filterEnable").toBool());
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
	settings.setValue("deckwindow/filterEnable", ui_.actionEnableFilter->isChecked());
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

void DeckWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

bool DeckWindow::event(QEvent* event)
{
	if (event->type() == QEvent::WindowActivate)
	{
		selectedCardChangedSlot();
	}
	return QMainWindow::event(event);
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
		double sumPriceLowest = 0.0;
		double sumPriceLowestFoil = 0.0;
		double sumPriceAverage = 0.0;
		double sumPriceTrend = 0.0;
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

			sumPriceLowest += quantity * getValue(row, mtg::ColumnType::PriceLowest).toDouble();
			sumPriceLowestFoil += quantity * getValue(row, mtg::ColumnType::PriceLowestFoil).toDouble();
			sumPriceAverage += quantity * getValue(row, mtg::ColumnType::PriceAverage).toDouble();
			sumPriceTrend += quantity * getValue(row, mtg::ColumnType::PriceTrend).toDouble();
		}
		QString message;
		QTextStream str(&message);
		str << "Showing " << model.rowCount() << " of " << deckWidget->deck().getNumRows() << " cards";
		str << " (" << numCopies << " copies, " << numLands << " lands, " << numCreatures << " creatures, " << numCopies - numCreatures - numLands << " others)";
		if (!deckWidget->isColumnHidden(mtg::ColumnType::PriceLowest))
		{
			str << " [Sum lowest price: " << sumPriceLowest << "]";
		}
		if (!deckWidget->isColumnHidden(mtg::ColumnType::PriceLowestFoil))
		{
			str << " [Sum lowest price foil: " << sumPriceLowestFoil << "]";
		}
		if (!deckWidget->isColumnHidden(mtg::ColumnType::PriceAverage))
		{
			str << " [Sum average price: " << sumPriceAverage << "]";
		}
		if (!deckWidget->isColumnHidden(mtg::ColumnType::PriceTrend))
		{
			str << " [Sum price trend: " << sumPriceTrend << "]";
		}
		ui_.statusBar->showMessage(message);
		ui_.actionToggleDeckActive->setChecked(deckWidget->deck().isActive());
	}
	else
	{
		ui_.statusBar->clearMessage();
		ui_.actionToggleDeckActive->setChecked(false);
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

	DeckWidget* deckWidget = new DeckWidget(filename);
	deckWidget->setHeaderState(headerState_);
	if (ui_.actionEnableFilter->isChecked())
	{
		deckWidget->setFilterRootNode(rootFilterNode_);
	}
	connect(deckWidget, SIGNAL(selectedCardChanged(int)), this, SIGNAL(selectedCardChanged(int)));
	connect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
	connect(deckWidget, SIGNAL(deckEdited()), this, SLOT(deckEdited()));
	connect(deckWidget, SIGNAL(searchStringChanged(QString)), permanentStatusBarLabel_, SLOT(setText(QString)));
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
				int ret = QMessageBox::question(this,
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
			disconnect(deckWidget, SIGNAL(searchStringChanged(QString)), permanentStatusBarLabel_, SLOT(setText(QString)));
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
			filename = QFileDialog::getSaveFileName(this, "Save Deck file", Settings::instance().getDecksDir(), "Decks (*.deck)");
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
		permanentStatusBarLabel_->clear();
	}
}

void DeckWindow::actionNewDeck()
{
	createDeckWidget();
}

void DeckWindow::actionOpenDeck()
{
	auto filename = QFileDialog::getOpenFileName(this, "Open Deck file", Settings::instance().getDecksDir(), "Decks (*.deck)");
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

void DeckWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	if (ui_.actionEnableFilter->isChecked())
	{
		for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
		{
			QWidget* widget = ui_.tabWidget->widget(tabIndex);
			if (widget)
			{
				if (ui_.actionEnableFilter->isChecked())
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
		emit addToCollection(static_cast<DeckWidget*>(widget)->deck().getQuantities());
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
			QMessageBox::information(this, "No selection", "No cards were selected. Please select the cards you want to make proxies for.");
			return;
		}

		auto fullBorders = QMessageBox::question(this, "Borders cropped?", "Would you like to use cropped or full borders?", "Cropped", "Full");

		auto pdfFile = QFileDialog::getSaveFileName(this, "Save to Pdf", QDir::homePath(), "Pdf (*.pdf)");
		if (pdfFile.isNull())
			return;

		QPdfWriter pdf(pdfFile);
		pdf.setPageSize(QPdfWriter::A4);
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
			auto pictureInfo = mtg::CardData::instance().getPictureInfo(index, fullBorders);
			if (pictureInfo.missing.empty())
			{
				for (int i = 0; i < quantity; ++i)
				{
					printCardLambda(pictureInfo.filenames.first());
					if (pictureInfo.layout == mtg::LayoutType::Transform)
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
			QMessageBox::information(this, "Success", "Proxy generation successful.");
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
		DeckStatisticsDialog dlg(deckWidget->deck(), this);
		dlg.exec();
	}
}

void DeckWindow::downloadCardArt()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->downloadCardArt();
	}
}

void DeckWindow::fetchOnlineData()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		deckWidget->fetchOnlineData();
	}
}

void DeckWindow::addToWantslist()
{
	DeckWidget* deckWidget = static_cast<DeckWidget*>(ui_.tabWidget->currentWidget());
	if (deckWidget)
	{
		auto indices = deckWidget->currentDataRowIndices();
		if (indices.isEmpty())
		{
			QMessageBox::information(this, "No selection", "No cards were selected. Please select the cards you want to add to a wantslist.");
			return;
		}
		try
		{
			deckWidget->addSelectionToWantslist();
		}
		catch (const mkm::MkmException& e)
		{
			QMessageBox msg(QMessageBox::Critical, "Error", e.getErrorMessage());
			msg.setDetailedText(e.getErrorDetails());
			msg.exec();
		}
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
			if (ui_.actionEnableFilter->isChecked())
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
