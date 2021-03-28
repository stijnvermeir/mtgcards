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
	const QString COLORS = "WUBRG";
	const QStringList TYPES = {"Creature", "Artifact", "Enchantment", "Instant", "Sorcery", "Land", "Planeswalker"};
	const QStringList COLOR_NAMES = {"White", "Blue", "Black", "Red", "Green", "Multicolor", "Colorless"};
	const QStringList RARITIES = {"Common", "Uncommon", "Rare", "Mythic Rare", "Special", "Basic Land"};

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

	auto lambda = [&](const Deck& deck, int row, int dataRow)
	{
		int cmc = mtg::CardData::instance().get(dataRow, mtg::ColumnType::CMC).toInt();
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
			for (const QString& color : COLOR_NAMES)
			{
				if (colors.contains(color))
				{
					colorCount[color] += q;
				}
			}
		}

		rarityCount[deck.get(row, mtg::ColumnType::Rarity).toString()] += q;

		// if not a land
		if (!types.contains("Land"))
		{
			if (deck.get(row, mtg::ColumnType::Layout).toString() == "split")
			{
				QString set = deck.get(row, mtg::ColumnType::SetCode).toString();
				for (const QString& name : deck.get(row, mtg::ColumnType::Names).toStringList())
				{
					int dataRow = mtg::CardData::instance().findRowFast(set, name);
					lambda(deck, row, dataRow);
				}
			}
			else
			{
				lambda(deck, row, deck.getDataRowIndex(row));
			}
		}

		auto tags = deck.get(row, mtg::ColumnType::Tags).toStringList();
		for (const QString& tag : tags)
		{
			tagsCount[tag] += q;
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
			str << color << ": " << colorCount[color] << " (" << 100.0 * colorCount[color] / numCards << " %)";
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
				str << " (" << (1.0 * cost[c] * 100 / total) << " %)";
			}
			QLabel* lbl = new QLabel(ManaCost::replaceTagsWithSymbols(text, MAGIC_SYMBOL_FONT_SIZE), this);
			ui_.manaColorsLayout->addWidget(lbl);
			ui_.manaColorsLayout->setAlignment(lbl, Qt::AlignHCenter);
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
}

DeckStatisticsDialog::~DeckStatisticsDialog()
{
}
