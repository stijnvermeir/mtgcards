#pragma once

#include <QString>

#include <memory>

class QHeaderView;

class Util
{
public:
	static QString saveHeaderViewState(QHeaderView& headerView);
	static void loadHeaderViewState(QHeaderView& headerView, const QString& data);

	static bool downloadPoolDataFile();

    static QString getOwnedAllTooltip(const int dataRowIndex);
    static QString getUsedTooltip(const int dataRowIndex);
    static QString getUsedAllTooltip(const int dataRowIndex);
};
