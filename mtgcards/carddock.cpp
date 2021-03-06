#include "carddock.h"
#include "settings.h"

#include "magiccarddata.h"

#include <QCloseEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>

namespace {

enum Page
{
	PAGE_PICTURE = 0,
	PAGE_MISSING = 1,
	PAGE_RULINGS = 2
};

} // namespace

CardDock::CardDock(Ui::MainWindow& ui, QWidget* parent)
    : QObject(parent)
    , ui_(ui)
	, scene_()
	, layoutType_(mtg::LayoutType::Normal)
	, imageFiles_()
	, secondViewActive_(false)
	, picturePage_(PAGE_PICTURE)
{
	ui_.cardView_->setScene(&scene_);
	ui_.cardView_->installEventFilter(this);
	ui_.label->installEventFilter(this);
	connect(ui_.backToPictureBtn, SIGNAL(clicked()), this, SLOT(switchPicture()));
	ui_.rulingsTbl->horizontalHeader()->setStretchLastSection(true);
}

CardDock::~CardDock()
{
}

bool CardDock::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
		if (mEvent->button() == Qt::LeftButton)
		{
			switchPicture();
		}
		else
		if (mEvent->button() == Qt::RightButton)
		{
			ui_.stackedWidget->setCurrentIndex(PAGE_RULINGS);
		}
		return true;
	}
	return QObject::eventFilter(object, event);
}

void CardDock::setCardPicture(const QString& imageFile, double rotation)
{
	scene_.clear();
	auto item = new QGraphicsPixmapItem(QPixmap(imageFile));
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setTransformOriginPoint(item->boundingRect().center());
	item->setRotation(rotation);
	scene_.addItem(item);
	ui_.cardView_->fitInView(item, Qt::KeepAspectRatio);
	ui_.cardView_->centerOn(item);
}

void CardDock::changeCardPicture(int row)
{
	auto picInfo = mtg::CardData::instance().getPictureInfo(row, Settings::instance().getArtIsHighQuality());
	imageFiles_ = picInfo.filenames;
	layoutType_ = picInfo.layout;
	secondViewActive_ = false;
	if (picInfo.missing.empty())
	{
		picturePage_ = PAGE_PICTURE;
		setCardPicture(imageFiles_.front(), 0);
	}
	else
	{
		picturePage_ = PAGE_MISSING;
		ui_.label->setText(QString("Could not find:\n") + picInfo.missing.join("\n"));
	}
	if (ui_.stackedWidget->currentIndex() != PAGE_RULINGS)
	{
		ui_.stackedWidget->setCurrentIndex(picturePage_);
	}

	const auto& rulings = mtg::CardData::instance().getRulings(row);
	ui_.rulingsTbl->clearContents();
	ui_.rulingsTbl->setRowCount(rulings.size());
	for (int i = 0; i < rulings.size(); ++i)
	{
		ui_.rulingsTbl->setItem(i, 0, new QTableWidgetItem(rulings[i].date));
		ui_.rulingsTbl->setItem(i, 1, new QTableWidgetItem(rulings[i].text));
	}
	ui_.rulingsTbl->resizeRowsToContents();
}

void CardDock::switchPicture()
{
	if (secondViewActive_)
	{
		setCardPicture(imageFiles_.front(), 0);
	}
	else
	{
		if (layoutType_ == mtg::LayoutType::Split)
		{
			setCardPicture(imageFiles_.front(), 90);
		}
		else
		if (layoutType_ == mtg::LayoutType::Aftermath)
		{
			setCardPicture(imageFiles_.front(), -90);
		}
		else
		if (layoutType_ == mtg::LayoutType::Transform || layoutType_ == mtg::LayoutType::ModalDFC)
		{
			setCardPicture(imageFiles_.back(), 0);
		}
		else
		if (layoutType_ == mtg::LayoutType::Flip)
		{
			setCardPicture(imageFiles_.front(), 180);
		}
	}
	secondViewActive_ = !secondViewActive_;
	ui_.stackedWidget->setCurrentIndex(picturePage_);
}
