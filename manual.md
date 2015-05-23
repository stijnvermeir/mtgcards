Manual
======

MTGCards is a tool to manage your collection and decks of Magic the Gathering cards.

First run
---------

When you run MTGCards for the first time, it will look bad. That's normal.
You'll have to configure some stuff before you can work with it properly.

### Card pool database
MTGCards does not embed the MTG card data into its binaries,
but requires the [AllSets.json file](http://mtgjson.com/json/AllSets.json.zip) from [mtgjson.com](mtgjson.com).

This means that every time WotC releases a new MTG set, you don't need to update the application! Yay!
Just replace your AllSets.json file with the latest version.

1. Download the latest version from the file and put it somewhere on your computer.
2. In Options, tab Data Sources, let MTGCards know where this file is located on your computer.

### Card preview images
MTGCards also does not include images of all the cards. You'll have to find them yourself somewhere ...

Once you find them, you should organize them in a specific way (described below)
and let MTGCards know in which directory they are located.
You can do that in the Data Sources tab in the Options.
