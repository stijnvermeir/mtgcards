#include "util.h"

#include <QHeaderView>
#include <QJsonDocument>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

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
