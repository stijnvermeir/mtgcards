#include "deckstatisticsdialog.h"
#include "manacurvelinewidget.h"
#include "manacost.h"
#include "magicconvert.h"
#include "deck.h"
#include "magiccarddata.h"

#include <QVector>
#include <QMap>
#include <QLabel>
#include <QDebug>

namespace
{
#ifndef Q_OS_OSX
const int MAGIC_SYMBOL_FONT_SIZE = 20;
#else
const int MAGIC_SYMBOL_FONT_SIZE = 24;
#endif
}

DeckStatisticsDialog::DeckStatisticsDialog(const Deck& deck, QWidget* parent)
	: QDialog(parent)
	, ui_()
{
	ui_.setupUi(this);

	const int MAX_CMC = 10;
	const QString COLORS = "WUBRGC";
	const QStringList TYPES = {"Creature", "Artifact", "Enchantment", "Instant", "Sorcery", "Land", "Planeswalker"};
	const QStringList COLOR_NAMES = {"White", "Blue", "Black", "Red", "Green", "Colorless", "Multicolor"};
	const QStringList RARITIES = {"Common", "Uncommon", "Rare", "Mythic Rare"};
	const QStringList RARITIES_CODE = {"common", "uncommon", "rare", "mythic"};

	int maxCmcCount = 0;
	int xCmcCount = 0;
	QVector<int> cmcCount(MAX_CMC + 1, 0);
	QMap<QChar, int> cost;
	int totalCards = 0;
	int totalCmc = 0;

	QMap<QString, int> typeCount;
	QMap<QString, int> colorCount;
	QMap<QString, int> rarityCount;
	QMap<QString, int> tagsCount;
	QMap<QString, int> categoriesCount;

	auto lambda = [&](const Deck& deck, int row, int dataRow)
	{
		int cmc = deck.get(row, mtg::ColumnType::CMC).toInt();
		if (cmc > MAX_CMC)
		{
			cmc = MAX_CMC;
		}
		int q = deck.get(row, mtg::ColumnType::Quantity).toInt();
		if (q > 0)
		{
			QString manaCost = mtg::toString(mtg::CardData::instance().get(dataRow, mtg::ColumnType::ManaCost));
			if (manaCost.contains("{X}"))
			{
				xCmcCount += q;
				if (xCmcCount > maxCmcCount)
				{
					maxCmcCount = xCmcCount;
				}
			}
			else
			{
				cmcCount[cmc] += q;
				if (cmcCount[cmc] > maxCmcCount)
				{
					maxCmcCount = cmcCount[cmc];
				}
			}
			for (const auto& c : COLORS)
			{
				cost[c] += manaCost.count(c) * q;
			}
			totalCards += q;
			totalCmc += cmc * q;
		}
	};

	QMap<QChar, int> manaSource;
	auto manaSourceLambda = [&](int row, int dataRow)
	{
		auto ms =  mtg::toString(mtg::CardData::instance().get(dataRow, mtg::ColumnType::ManaSource));
		int q = deck.get(row, mtg::ColumnType::Quantity).toInt();
		QMap<QChar, bool> isManaSource;
		for (QChar c : COLORS)
		{
			if (ms.contains(c))
			{
				manaSource[c] += q;
			}
		}
	};

	for (int row = 0; row < deck.getNumRows(); ++row)
	{
		QString types = deck.get(row, mtg::ColumnType::Type).toString();
		int q = deck.get(row, mtg::ColumnType::Quantity).toInt();
		for (const QString& type : TYPES)
		{
			if (types.contains(type))
			{
				typeCount[type] += q;
			}
		}

		if (!types.contains("Land"))
		{
			QStringList colors = deck.get(row, mtg::ColumnType::Color).toStringList();
			if (colors.empty())
			{
				colorCount["Colorless"] += q;
			}
			else
			if (colors.size() > 1)
			{
				colorCount["Multicolor"] += q;
			}
			else
			{
				for (QChar color : COLORS)
				{
					if (colors.contains(color))
					{
						colorCount[COLOR_NAMES[COLORS.indexOf(color)]] += q;
					}
				}
			}
		}

		int rarityIndex = RARITIES_CODE.indexOf(deck.get(row, mtg::ColumnType::Rarity).toString());
		if (rarityIndex != -1)
		{
			rarityCount[RARITIES[rarityIndex]] += q;
		}

		mtg::LayoutType layout(deck.get(row, mtg::ColumnType::Layout).toString());
		if (layout.hasMultipleNames())
		{
			QString set = deck.get(row, mtg::ColumnType::SetCode).toString();
			for (const QString& name : deck.get(row, mtg::ColumnType::Names).toStringList())
			{
				int dataRow = mtg::CardData::instance().findRowFast(set, name);
				if (!types.contains("Land"))
				{
					lambda(deck, row, dataRow);
				}
				manaSourceLambda(row, dataRow);
			}
		}
		else
		{
			if (!types.contains("Land"))
			{
				lambda(deck, row, deck.getDataRowIndex(row));
			}
			manaSourceLambda(row, deck.getDataRowIndex(row));
		}

		auto tags = deck.get(row, mtg::ColumnType::Tags).toStringList();
		for (const QString& tag : tags)
		{
			tagsCount[tag] += q;
		}

		auto categories = deck.get(row, mtg::ColumnType::Categories).toStringList();
		for (const QString& cat : categories)
		{
			categoriesCount[cat] += q;
		}
	}

	int numCards = deck.getNumCards();

	// colors
	{
		for (const QString& color : COLOR_NAMES)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << color << ": " << colorCount[color] << " (" << 100.0 * colorCount[color] / totalCards << " %)";
			ui_.colorsLayout->addWidget(new QLabel(text, this));
		}
	}

	// card types
	{
		for (const QString& type : TYPES)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << type << ": " << typeCount[type] << " (" << 100.0 * typeCount[type] / numCards << " %)";
			ui_.cardTypesLayout->addWidget(new QLabel(text, this));
		}
	}

	// rarity
	{
		for (const QString& rarity : RARITIES)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << rarity << ": " << rarityCount[rarity] << " (" << 100.0 * rarityCount[rarity] / numCards << " %)";
			ui_.rarityLayout->addWidget(new QLabel(text, this));
		}
	}

	// mana curve
	{
		for (int i = 0; i <= MAX_CMC; ++i)
		{
			ManaCurveLineWidget* w = new ManaCurveLineWidget(this);
			QString cmc = QString("{") + QString::number(i) + "}";
			w->setCmc(cmc);
			w->setMax(maxCmcCount);
			w->setCount(cmcCount[i]);
			ui_.manaCurveLayout->addWidget(w);
		}
		ManaCurveLineWidget* w = new ManaCurveLineWidget(this);
		w->setCmc("{X}");
		w->setMax(maxCmcCount);
		w->setCount(xCmcCount);
		ui_.manaCurveLayout->addWidget(w);
	}

	// mana colors
	{
		int total = 0;
		for (const auto& c : COLORS)
		{
			total += cost[c];
		}
		for (const auto& c : COLORS)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << "{" << c << "}" << " x" << cost[c];
			if (total > 0)
			{
				str << " (" << (1.0 * cost[c] * 100 / total) << "%)";
			}
			QLabel* lbl = new QLabel(ManaCost::replaceTagsWithSymbols(text, MAGIC_SYMBOL_FONT_SIZE), this);
			ui_.manaColorsLayout->addWidget(lbl);
			ui_.manaColorsLayout->setAlignment(lbl, Qt::AlignLeft);
		}
	}
	// mana sources
	{
		int total = numCards;
		for (const auto& c : COLORS)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << "{" << c << "}" << " x" << manaSource[c];
			if (total > 0)
			{
				str << " (" << (1.0 * manaSource[c] * 100 / total) << "%)";
			}
			QLabel* lbl = new QLabel(ManaCost::replaceTagsWithSymbols(text, MAGIC_SYMBOL_FONT_SIZE), this);
			ui_.manaSourcesLayout->addWidget(lbl);
			ui_.manaSourcesLayout->setAlignment(lbl, Qt::AlignLeft);
		}
	}

	// mana breakdown
	{
		const int MAX_ROWS = 3;
		int row = 0;
		int col = 0;
		auto addWidgetLambda = [&](const QString& text)
		{
			ui_.manaBreakdownLayout->addWidget(new QLabel(ManaCost::replaceTagsWithSymbols(text, MAGIC_SYMBOL_FONT_SIZE), this), row, col);
			++row;
			if (row >= MAX_ROWS)
			{
				row = 0;
				++col;
			}
		};
		if (totalCards == 0)
		{
			totalCards = 1;
		}

		// colors
		for (const auto& c : COLORS)
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << "{" << c << "} " << 1.0 * cost[c] / totalCards;
			str << " (" << 1.0 * cost[c] * 100 / totalCmc << " %)";
			addWidgetLambda(text);
		}

		// colorless
		{
			int colorlessCost = totalCmc;
			for (const auto& c : COLORS)
			{
				colorlessCost -= cost[c];
			}
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << "{1} " << 1.0 * colorlessCost / totalCards;
			str << " (" << 1.0 * colorlessCost * 100 / totalCmc << " %)";
			addWidgetLambda(text);
		}

		// total cards
		{
			QString text;
			QTextStream str(&text);
			str << "Total cards: " << totalCards;
			addWidgetLambda(text);
		}

		// total mana
		{
			QString text;
			QTextStream str(&text);
			str << "Total CMC: " << totalCmc;
			addWidgetLambda(text);
		}

		// total mana
		{
			QString text;
			QTextStream str(&text);
			str.setRealNumberPrecision(2);
			str.setRealNumberNotation(QTextStream::FixedNotation);
			str << "Average CMC: " << 1.0 * totalCmc / totalCards;
			addWidgetLambda(text);
		}
	}

	// Tags stats
	{
		ui_.tagsTbl->setRowCount(tagsCount.size());
		int r = 0;
		auto tag = tagsCount.constBegin();
		while (tag != tagsCount.constEnd())
		{
			ui_.tagsTbl->setItem(r, 0, new QTableWidgetItem(tag.key()));
			auto item = new QTableWidgetItem(QString::number(tag.value()));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui_.tagsTbl->setItem(r, 1, item);
			++tag;
			++r;
		}
		ui_.tagsTbl->resizeColumnsToContents();

	}

	// Categories stats
	{
		ui_.categoriesTbl->setRowCount(categoriesCount.size());
		int r = 0;
		auto cat = categoriesCount.constBegin();
		while (cat != categoriesCount.constEnd())
		{
			ui_.categoriesTbl->setItem(r, 0, new QTableWidgetItem(cat.key()));
			auto item = new QTableWidgetItem(QString::number(cat.value()));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			ui_.categoriesTbl->setItem(r, 1, item);
			++cat;
			++r;
		}
		ui_.categoriesTbl->resizeColumnsToContents();

	}
}

DeckStatisticsDialog::~DeckStatisticsDialog()
{
}
