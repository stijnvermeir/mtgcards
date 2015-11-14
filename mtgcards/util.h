#pragma once

#include <mkm/mkm.h>

#include <QString>

#include <memory>

class QHeaderView;

class Util
{
public:
	static QString saveHeaderViewState(QHeaderView& headerView);
	static void loadHeaderViewState(QHeaderView& headerView, const QString& data);

	static std::unique_ptr<mkm::Mkm> mkmClient();
};
