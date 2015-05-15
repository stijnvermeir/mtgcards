#include "deckwindow.h"

#include "deckwidget.h"
#include "deck.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "filtereditordialog.h"
#include "settings.h"
#include "util.h"

#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QPdfWriter>
#include <QPainter>
#include <QDebug>

DeckWindow::DeckWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_()
	, headerState_()
	, rootFilterNode_()
{
	setWindowFlags(Qt::NoDropShadowWindowHint);
	ui_.setupUi(this);

	connect(ui_.actionNewDeck, SIGNAL(triggered()), this, SLOT(actionNewDeck()));
	connect(ui_.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeDeck(int)));
	connect(ui_.actionOpenDeck, SIGNAL(triggered()), this, SLOT(actionOpenDeck()));
	connect(ui_.actionSaveDeck, SIGNAL(triggered()), this, SLOT(actionSaveDeck()));
	connect(ui_.actionSaveDeckAs, SIGNAL(triggered()), this, SLOT(actionSaveDeckAs()));
	connect(ui_.actionAdvancedFilter, SIGNAL(triggered()), this, SLOT(actionAdvancedFilter()));
	connect(ui_.actionAddToCollection, SIGNAL(triggered()), this, SLOT(actionAddToCollection()));
	connect(ui_.actionRemoveFromCollection, SIGNAL(triggered()), this, SLOT(actionRemoveFromCollection()));
	connect(ui_.actionAddToDeck, SIGNAL(triggered()), this, SLOT(actionAddToDeck()));
	connect(ui_.actionRemoveFromDeck, SIGNAL(triggered()), this, SLOT(actionRemoveFromDeck()));
	connect(ui_.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChangedSlot(int)));
	connect(ui_.actionToggleDeckActive, SIGNAL(triggered(bool)), this, SLOT(actionToggleDeckActive(bool)));
	connect(ui_.actionCreateProxies, SIGNAL(triggered()), this, SLOT(createProxies()));
}

DeckWindow::~DeckWindow()
{
}

void DeckWindow::reload()
{
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		QWidget* widget = ui_.tabWidget->widget(tabIndex);
		if (widget)
		{
			static_cast<DeckWidget*>(widget)->reload();
		}
	}
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
	if (settings.contains("deckwindow/filter"))
	{
		rootFilterNode_ = FilterNode::createFromJson(QJsonDocument::fromJson(settings.value("deckwindow/filter").toString().toUtf8()));
	}
	if (settings.contains("deckwindow/headerstate"))
	{
		headerState_ = settings.value("deckwindow/headerstate").toString();
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

void DeckWindow::closeEvent(QCloseEvent* event)
{
	emit windowClosed(false);
	event->accept();
}

bool DeckWindow::event(QEvent* event)
{
	if (event->type() == QEvent::WindowActivate /*|| event->type() == QEvent::Enter*/)
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
		auto getValue = [&model](int row, mtg::ColumnType columnType)
		{
			int column = model.columnToIndex(columnType);
			QModelIndex index = model.sourceModel()->index(row, column);
			return model.sourceModel()->data(index);
		};
		int landCount = 0;
		int creatureCount = 0;
		int cardCount = 0;
		for (int row = 0; row < model.sourceModel()->rowCount(); ++row)
		{
			int quantity = getValue(row, mtg::ColumnType::Quantity).toInt();
			QString type = getValue(row, mtg::ColumnType::Type).toString();
			if (type.contains("Creature"))
			{
				creatureCount += quantity;
			}
			if (type.contains("Land"))
			{
				landCount += quantity;
			}
			cardCount += quantity;
		}
		QString message;
		QTextStream str(&message);
		str << cardCount << " cards (" << landCount << " lands, " << creatureCount << " creatures, " << cardCount - creatureCount - landCount << " others)";
		ui_.statusBar->showMessage(message);
		ui_.actionToggleDeckActive->setChecked(deckWidget->deck().isActive());
	}
	else
	{
		ui_.actionToggleDeckActive->setChecked(false);
		ui_.statusBar->clearMessage();
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
	deckWidget->setFilterRootNode(rootFilterNode_);
	connect(deckWidget, SIGNAL(selectedCardChanged(int)), this, SIGNAL(selectedCardChanged(int)));
	connect(deckWidget, SIGNAL(headerStateChangedSignal(QString)), this, SLOT(headerStateChangedSlot(QString)));
	connect(deckWidget, SIGNAL(deckEdited()), this, SLOT(deckEdited()));
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

void DeckWindow::currentTabChangedSlot(int)
{
	selectedCardChangedSlot();
	updateStatusBar();
}

void DeckWindow::actionNewDeck()
{
	createDeckWidget();
}

void DeckWindow::closeDeck(int index)
{
	destroyDeckWidget(static_cast<DeckWidget*>(ui_.tabWidget->widget(index)));
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

void DeckWindow::actionAdvancedFilter()
{
	FilterEditorDialog editor;
	editor.setFilterRootNode(rootFilterNode_);
	editor.exec();
	rootFilterNode_ = editor.getFilterRootNode();
	for (int tabIndex = 0; tabIndex < ui_.tabWidget->count(); ++tabIndex)
	{
		QWidget* widget = ui_.tabWidget->widget(tabIndex);
		if (widget)
		{
			static_cast<DeckWidget*>(widget)->setFilterRootNode(rootFilterNode_);
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

		for (int index : indices)
		{
			auto quantity = deckWidget->deck().getQuantity(index);
			auto pictureData = mtg::CardData::instance().getPictureFilenames(index);
			for (int i = 0; i < quantity; ++i)
			{
				printCardLambda(pictureData.second.first());
				if (pictureData.first == mtg::LayoutType::DoubleFaced)
				{
					printCardLambda(pictureData.second.last());
				}
			}
		}

		painter.end();

		QMessageBox::information(this, "Success", "Proxy generation successful.");
	}
}

void DeckWindow::handleOpenDeckRequest(const QString& deckId)
{
	createDeckWidget(deckId);
}
