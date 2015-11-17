#include "addtowantslistdialog.h"
#include "ui_addtowantslistdialog.h"
#include "magiccarddata.h"

#include <QInputDialog>
#include <QProgressDialog>

AddToWantslistDialog::AddToWantslistDialog(const QVector<int>& selection, QWidget* parent)
	: QDialog(parent)
	, ui_(new Ui::AddToWantslistDialog)
	, selection_(selection)
{
	ui_->setupUi(this);

	loadWantslists(Util::mkmClient()->getWantsLists());
	connect(ui_->newWantslistBtn, SIGNAL(clicked()), this, SLOT(addWantslistClicked()));

	ui_->minConditionCbx->addItem("Mint", mkm::Want::Mint);
	ui_->minConditionCbx->addItem("Near mint", mkm::Want::NearMint);
	ui_->minConditionCbx->addItem("Excellent", mkm::Want::Excellent);
	ui_->minConditionCbx->addItem("Good", mkm::Want::Good);
	ui_->minConditionCbx->addItem("Light played", mkm::Want::LightPlayed);
	ui_->minConditionCbx->addItem("Played", mkm::Want::Played);
	ui_->minConditionCbx->addItem("Poor", mkm::Want::Poor);

	connect(ui_->buttonBox, SIGNAL(accepted()), this, SLOT(addWants()));
	connect(ui_->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AddToWantslistDialog::~AddToWantslistDialog()
{
	delete ui_;
}

void AddToWantslistDialog::loadWantslists(const QVector<mkm::Wantslist>& wantslists)
{
	ui_->wantslistCbx->clear();
	for (const mkm::Wantslist& wantslist : wantslists)
	{
		if (wantslist.idGame == 1)
		{
			ui_->wantslistCbx->addItem(QString("%1 (%2)").arg(wantslist.name).arg(wantslist.itemCount), wantslist.idWantslist);
		}
	}
}

void AddToWantslistDialog::addWantslistClicked()
{
	QString name = QInputDialog::getText(this, "New wantslist", "Name:");
	if (!name.isNull())
	{
		loadWantslists(Util::mkmClient()->addWantsList(name));
	}
}

void AddToWantslistDialog::addWants()
{
	int wantsListId = ui_->wantslistCbx->currentData().toInt();
	QVector<mkm::Want> wants;
	QProgressDialog progress("Fetching online data ...", "Cancel", 0, selection_.size());
	progress.setWindowModality(Qt::WindowModal);
	int i = 0;
	for (int dataRowIndex : selection_)
	{
		QVariant metaProductid = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::MkmMetaproductId);
		QVariant productid = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::MkmProductId);
		if (metaProductid.isNull() || productid.isNull()) // not known -> try fetching
		{
			mtg::CardData::instance().fetchOnlineData(dataRowIndex);
			metaProductid = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::MkmMetaproductId);
			productid = mtg::CardData::instance().get(dataRowIndex, mtg::ColumnType::MkmProductId);
		}
		if (metaProductid.isNull() || productid.isNull()) // still not known -> skip
		{
			continue;
		}
		mkm::Want w;
		if (ui_->fromAnySetChk->isChecked())
		{
			w.metaProductId = metaProductid.toInt();
		}
		else
		{
			w.productId = productid.toInt();
		}
		w.amount = ui_->countSbx->value();
		w.minCondition = static_cast<mkm::Want::Condition>(ui_->minConditionCbx->currentData().toInt());
		w.buyPrice = ui_->wishPriceSbx->value();
		w.languageIds.push_back(1);
		wants.push_back(w);
		progress.setValue(i++);
		if (progress.wasCanceled())
		{
			break;
		}
	}
	progress.setValue(selection_.size());
	if (!progress.wasCanceled())
	{
		Util::mkmClient()->addWants(wantsListId, wants);
		accept();
	}
}
