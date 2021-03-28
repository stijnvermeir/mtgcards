#include "util.h"
#include "magiccarddata.h"
#include "magiccollection.h"
#include "deckmanager.h"
#include "settings.h"

#include <QHeaderView>
#include <QJsonDocument>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QtDebug>

#include "bzip2/bzlib.h"

namespace {

QString logicalIndexToName(QHeaderView& view, int logicalIndex)
{
	return view.model()->headerData(logicalIndex, Qt::Horizontal, Qt::UserRole).toString();
}

QVariant nameToLogicalIndex(QHeaderView& view, const QString& name)
{
	for (int i = 0; i < view.count(); ++i)
	{
		auto n = view.model()->headerData(i, Qt::Horizontal, Qt::UserRole);
		if (n.isValid() && n.toString() == name)
		{
			return i;
		}
	}
	return QVariant();
}

}

QString Util::saveHeaderViewState(QHeaderView& headerView)
{
	QVariantMap headerState;
	headerState["sortIndicatorSection"] = logicalIndexToName(headerView, headerView.sortIndicatorSection());
	headerState["sortIndicatorOrder"] = headerView.sortIndicatorOrder();
	QVariantMap sections;
	for (int logicalIndex = 0; logicalIndex < headerView.count(); ++logicalIndex)
	{
		QVariantMap section;
		section["visualIndex"] = headerView.visualIndex(logicalIndex);
		bool hidden = headerView.isSectionHidden(logicalIndex);
		headerView.showSection(logicalIndex);
		section["size"] = headerView.sectionSize(logicalIndex);
		headerView.setSectionHidden(logicalIndex, hidden);
		section["hidden"] = hidden;
		sections.insert(logicalIndexToName(headerView, logicalIndex), section);
	}
	headerState["sections"] = sections;
	QString data = QJsonDocument::fromVariant(headerState).toJson(QJsonDocument::Compact);
	return data;
}

void Util::loadHeaderViewState(QHeaderView& headerView, const QString& data)
{
	QVariantMap headerState = QJsonDocument::fromJson(data.toUtf8()).toVariant().toMap();
	QVariantMap sections = headerState["sections"].toMap();
	for (int logicalIndex = 0; logicalIndex < headerView.count(); ++logicalIndex)
	{
		auto sectionName = logicalIndexToName(headerView, logicalIndex);
		if (sections.contains(sectionName))
		{
			QVariantMap section = sections[sectionName].toMap();
			int currentVisualIndex = headerView.visualIndex(logicalIndex);
			int newVisualIndex = section["visualIndex"].toInt();
            headerView.swapSections(currentVisualIndex, newVisualIndex);
			int size = section["size"].toInt();
			if (size <= 0)
			{
				size = headerView.defaultSectionSize();
			}
			headerView.resizeSection(logicalIndex, size);
            headerView.setSectionHidden(logicalIndex, section["hidden"].toBool());
		}
	}
	auto sortSection = nameToLogicalIndex(headerView, headerState["sortIndicatorSection"].toString());
	if (sortSection.isValid())
	{
		Qt::SortOrder sortOrder = static_cast<Qt::SortOrder>(headerState["sortIndicatorOrder"].toInt());
		headerView.setSortIndicator(sortSection.toInt(), sortOrder);
	}
}

bool Util::downloadPoolDataFile()
{
	QNetworkAccessManager network;
	QNetworkRequest request;
	QString whichFile = "AllPrintings.sqlite";
	request.setUrl(QString("https://mtgjson.com/api/v5/%1").arg(whichFile));
	QScopedPointer<QNetworkReply> reply(network.get(request));
	QEventLoop loop;
	QProgressDialog progress("Downloading ...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.show();
	QObject::connect(reply.data(), &QNetworkReply::downloadProgress,
	[&progress, &whichFile](qint64 bytesReceived, qint64 bytesTotal)
	{
		progress.setLabelText(QString("<p><b>Downloading %1 ...</b></p><p>Received %2 of %3 KiB</p>").arg(whichFile).arg(bytesReceived/1024).arg(bytesTotal/1024));
		progress.setMaximum(bytesTotal);
		progress.setValue(bytesReceived);
	});
	QObject::connect(&progress, &QProgressDialog::canceled, reply.data(), &QNetworkReply::abort);
	QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	if (reply->error())
	{
		QMessageBox::critical(nullptr, "Download error", reply->errorString());
		return false;
	}

	QFile file(Settings::instance().getPoolDataFile());
	QFileInfo fi(file);
	fi.absoluteDir().mkpath(fi.absolutePath());
	file.open(QIODevice::WriteOnly);
	file.write(reply->readAll());
	return true;
}

bool Util::downloadPricesFile()
{
	QNetworkAccessManager network;
	QNetworkRequest request;
	QString whichFile = "AllPrices.json.bz2";
	request.setUrl(QString("https://mtgjson.com/api/v5/%1").arg(whichFile));
	QScopedPointer<QNetworkReply> reply(network.get(request));
	QEventLoop loop;
	QProgressDialog progress("Downloading ...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.show();
	QObject::connect(reply.data(), &QNetworkReply::downloadProgress,
	[&progress, &whichFile](qint64 bytesReceived, qint64 bytesTotal)
	{
		progress.setLabelText(QString("<p><b>Downloading %1 ...</b></p><p>Received %2 of %3 KiB</p>").arg(whichFile).arg(bytesReceived/1024).arg(bytesTotal/1024));
		progress.setMaximum(bytesTotal);
		progress.setValue(bytesReceived);
	});
	QObject::connect(&progress, &QProgressDialog::canceled, reply.data(), &QNetworkReply::abort);
	QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();
	if (reply->error())
	{
		QMessageBox::critical(nullptr, "Download error", reply->errorString());
		return false;
	}

	QFile file(Settings::instance().getPricesBz2File());
	QFileInfo fi(file);
	fi.absoluteDir().mkpath(fi.absolutePath());
	file.open(QIODevice::WriteOnly);
	file.write(reply->readAll());
	return true;
}

bool Util::decompressPricesFile()
{
	FILE*   f;
	BZFILE* b;
	int     nBuf;
	const size_t BUF_SIZE = 1024;
	char    buf[BUF_SIZE];
	int     bzerror;
	FILE*   fOut;

	auto bz2File = Settings::instance().getPricesBz2File().toStdString();
	qDebug() << bz2File.c_str();
	fopen_s(&f, bz2File.c_str(), "rb");
	if (!f)
	{
		qDebug() << "Failed to open bz2 input file";
		return false;
	}
	fopen_s(&fOut, Settings::instance().getPricesJsonFile().toStdString().c_str(), "wb");
	if (!fOut)
	{
		qDebug() << "Failed to open json output file";
		fclose(f);
		return false;
	}
	b = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);
	if (bzerror != BZ_OK)
	{
		qDebug() << "BZ2_bzReadOpen failed: " << bzerror;
		BZ2_bzReadClose(&bzerror, b);
		fclose(f);
		fclose(fOut);
		return false;
	}
	bzerror = BZ_OK;
	while (bzerror == BZ_OK)
	{
		nBuf = BZ2_bzRead(&bzerror, b, buf, BUF_SIZE);
		if (bzerror == BZ_OK)
		{
			fwrite(buf, 1, nBuf, fOut);
		}
	}
	if (bzerror != BZ_STREAM_END)
	{
		qDebug() << "BZ2_bzRead failed: " << bzerror;
		BZ2_bzReadClose(&bzerror, b);
		fclose(f);
		fclose(fOut);
		return false;
	}
	BZ2_bzReadClose(&bzerror, b);
	fclose(f);
	fclose(fOut);
	return true;
}

QString Util::getOwnedAllTooltip(const int dataRowIndex)
{
    const auto& reprintRowIndicesInData = mtg::CardData::instance().findReprintRows(dataRowIndex);
    QStringList tooltip;
    for (const auto& i : reprintRowIndicesInData)
    {
        int qty = mtg::Collection::instance().getQuantity(i);
        if (qty > 0)
        {
            QString tooltipLine;
            QTextStream str(&tooltipLine);
            str << qty << "x from " << mtg::CardData::instance().get(i, mtg::ColumnType::SetCode).toString();
            tooltip << tooltipLine;
        }
    }
    if (tooltip.empty())
    {
        return 0;
    }
    return tooltip.join("\n");
}

QString Util::getUsedTooltip(const int dataRowIndex)
{
    QStringList tooltip;
    auto decks = DeckManager::instance().getDecksUsedIn(dataRowIndex);
    for (const auto& deck : decks)
    {
        QString tooltipLine;
        QTextStream str(&tooltipLine);
        str << deck->getQuantity(dataRowIndex) << "x in " << deck->getDisplayName();
        tooltip << tooltipLine;
    }
    if (tooltip.empty())
    {
        return "Not used.";
    }
    return tooltip.join("\n");
}

QString Util::getUsedAllTooltip(const int dataRowIndex)
{
    const auto& reprintRowIndicesInData = mtg::CardData::instance().findReprintRows(dataRowIndex);
    QStringList tooltip;
    for (const auto& i : reprintRowIndicesInData)
    {
        auto decks = DeckManager::instance().getDecksUsedIn(i);
        for (const auto& deck : decks)
        {
            QString tooltipLine;
            QTextStream str(&tooltipLine);
            str << deck->getQuantity(i) << "x from " << mtg::CardData::instance().get(i, mtg::ColumnType::SetCode).toString() << " in " << deck->getDisplayName();
            tooltip << tooltipLine;
        }
    }
    if (tooltip.empty())
    {
        return "Not used.";
    }
    return tooltip.join("\n");
}
