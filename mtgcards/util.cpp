#include "util.h"
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
			headerView.moveSection(currentVisualIndex, newVisualIndex);
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
	int rv = QMessageBox::question(nullptr, "Pool data", "<p style=\"font-weight: normal;\"><b>MTGCards uses <a href=\"http://mtgjson.com\">mtgjson.com</a> for its card database. "
														 "Do you want to download the latest Allsets.json (without rulings) or AllSets-x.json (with rulings) file now?</b> "
														 "<i>You can also skip this step and download the file yourself. You can point to it in the MTGCards Options.</i></p>",
								   "Skip", "Download without rulings", "Download with rulings", 2, 0);
	if (rv == 0)
	{
		return false;
	}

	QNetworkAccessManager network;
	QNetworkRequest request;
	QString whichFile;
	if (rv == 1)
	{
		whichFile = "AllSets.json";
	}
	else
	{
		whichFile = "AllSets-x.json";
	}
	request.setUrl(QString("http://mtgjson.com/json/%1").arg(whichFile));
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
