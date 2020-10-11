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

QString Util::saveHeaderViewState(QHeaderView& headerView)
{
	QVariantMap headerState;
	headerState["sortIndicatorSection"] = headerView.sortIndicatorSection();
	headerState["sortIndicatorOrder"] = headerView.sortIndicatorOrder();
	QVariantList sections;
	for (int logicalIndex = 0; logicalIndex < headerView.count(); ++logicalIndex)
	{
		QVariantMap section;
		section["visualIndex"] = headerView.visualIndex(logicalIndex);
		bool hidden = headerView.isSectionHidden(logicalIndex);
		headerView.showSection(logicalIndex);
		section["size"] = headerView.sectionSize(logicalIndex);
		headerView.setSectionHidden(logicalIndex, hidden);
		section["hidden"] = hidden;
		sections.append(section);
	}
	headerState["sections"] = sections;
	QString data = QJsonDocument::fromVariant(headerState).toJson(QJsonDocument::Compact);
	return data;
}

void Util::loadHeaderViewState(QHeaderView& headerView, const QString& data)
{
	QVariantMap headerState = QJsonDocument::fromJson(data.toUtf8()).toVariant().toMap();
	QVariantList sections = headerState["sections"].toList();
	for (int logicalIndex = 0; logicalIndex < sections.size(); ++logicalIndex)
	{
		QVariantMap section = sections[logicalIndex].toMap();
		if (logicalIndex < headerView.count())
		{
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
	int sortSection = headerState["sortIndicatorSection"].toInt();
	if (sortSection < headerView.count())
	{
		Qt::SortOrder sortOrder = static_cast<Qt::SortOrder>(headerState["sortIndicatorOrder"].toInt());
		headerView.setSortIndicator(sortSection, sortOrder);
	}
}

std::unique_ptr<mkm::Mkm> Util::mkmClient()
{
	static const auto& settings = Settings::instance().getMkm();
	return std::unique_ptr<mkm::Mkm>(new mkm::Mkm(settings.getEndpoint() , settings.appToken, settings.appSecret, settings.accessToken, settings.accessTokenSecret));
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
