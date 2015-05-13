#pragma once

#include <QString>

class QHeaderView;

class Util
{
public:
	static QString saveHeaderViewState(QHeaderView& headerView);
	static void loadHeaderViewState(QHeaderView& headerView, const QString& data);
};
