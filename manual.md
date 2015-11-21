Manual
======

MTGCards is a tool to manage your collection and decks of Magic the Gathering cards.

First run
---------

### 1. Card pool database
MTGCards does not embed the MTG card data into its binaries,
but requires the AllSets.json file from <http://mtgjson.com>.

This means that every time WotC releases a new MTG set, you don't need to update the application! Yay!

On the first run, you're asked to download this file from within MTGCards. It's the easiest way to get started. You can choose to download the data with or without rulings from the Gatherer.

Later, whenever mtgjson.com releases a new version, you can download the latest version from within MTGCards by going to Options, tab Data Sources and click "Download latest ...".

### 2. Card preview images
MTGCards does not include images of all the cards. By default, they are downloaded from the gatherer website whenever you select a card. You can disable this feature in the Options, under the Misc tab.

If you want high resolution scans, you'll need to make or find them yourself ... .
Once you got them, you should organize them in a specific way (described below)
and let MTGCards know in which folder they are located.
You can do that in the Data Sources tab in the Options.

#### Card Image Folder Structure and naming convention
You should have a folder per Set. The name of a Set folder should be the same as the value in the *Set Name* column for a card.

Inside a Set folder, you need a .jpg file for each card. The name of this file should be the same as the value in the *Name* column for a card, appended with the .jpg extension.

Any special character in the name of a Set or a card should be replaced by correspending ASCII characters.
In addition there are some more special cases for naming card image files.

##### Example
Let's say you are looking at *Lim-Dûl the Necromancer* from the *Time Spiral* Set in the Card Pool window. You don't see a preview image.

1. Create a folder name *Time Spiral* inside your image folder.
2. Copy the image of the card inside this folder and rename the file to *Lim-Dul the Necromancer.jpg*.

##### Split and flip cards
Split and flip cards have more than one name. Concatenate the names with underscores between them.
For example *Give/Take* would become *Give_Take.jpg*.

##### Tokens
Images for tokens should be inside a folder named *token* inside the Set folder. The image should be the token name.

##### Cards with multiple versions (like Basic Lands)
The filenames of such cards should have an extra version tag. This is a number between square brackets.

For example, one version of a Forest inside a Set should have the name *Forest [1].jpg*. Another one should be *Forest [2].jpg*. And so on.

### 3. Window position, size and visibility
The windows will have a default position and size when running for the first time. Adjust their position and size to your liking. Their state will be saved at exit.

### 4. Column width, order and visibility
The columns will have default sizes and the most interesting ones will be visible on the first run. Resize and move them to your liking. You can hide or unhide columns by right-clicking on the header. Their state will be saved at exit.

Features
--------

### Automatic use count tracking of cards in collection
In the Collection window you will see the *Used* column. This value is calculated from *active* decks that use this card.

A deck can be activated or deactivated by toggling the switch in the Deck window.

When you hover over a *Used* cell, you can see how many times the card is used in which deck. You can right click on the row and select *Open decks where this card is used* to open them.

For this feature to work properly, the .deck files should be kept in the *decks* folder inside the MTGCards Application Folder. Where this folder is located, can be found in the Options under Misc. It can also be changed there.

### Advanced Filter
You can set a filter per window (Pool, Collection and Deck). You can create an entire filter tree and group filters in **AND** and **OR** groups. There are different filter types:

1. **Regex** Regular expression support (very powerful)
2. **Number** Compare numeric fields against a number
3. **Date** Compare date fields against a date
4. **Time** Compare timestamp fields against a time
5. **Deck** Check if a card is present in a deck

Each filter can be negated to have the opposite effect. For example, with the Deck filter, you can either show only the cards that are in a certain deck or show all cards that are not in certain deck.

### magiccardmarket.eu integration
You can fetch prices for selected cards by clicking *the cloud icon*.

In the Deck window, you can add selected cards to a wantslist by clicking *the heart icon*.

In order for [magiccardmarket.eu](http://magiccardmarket.eu) integration to work, you'll need an account there and add an app to your profile. Adding an app to your profile will give you an App token, App secret, Access token and Access token secret. Once you enter this information in the Options, under magiccardmarket.eu tab, you're ready to go.

### Proxy generation
In the Deck window, you can select multiple cards. When you click *the printer icon*, choose a name for the output PDF. The selected cards will be printed and cards will be printed as many times as they are used in the deck.

### Keyboard search
After selecting a cell in Pool, Collection or Card window, you can just start typing and the application will search for the first matching cell in that column.

Select a different cell to reset the keyboard search.

Start your search with ^ and it will only match if the name *starts* with the search string.

Use PgUp and PgDn to go to the previous or next match.

### Deck statistics
Click on *the graph icon* in the Deck window to see statistics about your deck.

### Card pictures
For double sided cards, you can click on the card image to see the back.

For flipped or split cards, you can click on the card image to flip or rotate the image.

### Card rulings
When you right click on the card's image, the view will change to *Rulings*. These will only be available when you chose to download the card data file with rulings.

### Customization
MTGCards aims to be very customizable.

#### Multi display support
Every element of the application has its own window. This way it's a piece of cake to put your Deck window on one screen and the Pool window on another.

#### Keyboard shortcuts
You can change the default keyboard shortcuts in the Options under *Shortcuts*.

#### User columns
You can define your own columns in the Options under *User Columns*. After a restart of the application, your columns will become available in the Collection and Deck window. You can also use them in filters.

### Import collection and decks

#### Import collection
You can import your collection from a .csv file in UTF-8 encoding. The expected format is: 3-letter set code;name;quantity.

For example:

	M12;Acidic Slime;4
	ISD;Mikaeus, The Lunarch;1

Only the official 3-letter codes are recognized. You will have to manually fix them if your current collection uses different codes.

Special characters in card names should not be replaced.

#### Import deck
You can import multiple decks from UTF-8 encoded XML files. MTGCards will look for *card* elements.

XML attributes:

* **edition** Official 3-letter set code
* **deck** The quantity of that card in the deck
* **sb** The amount that card is on the Sideboard

XML card element content should be the card name.

For example:

	<card edition="M12" deck="1" sb="0">Acidic Slime</card>
	<card edition="ISD" deck="0" sb="1">Mikaeus, The Lunarch</card>
	
### Multi-platform
MTGCards is a native desktop application available for Mac OS X, Windows and Linux.

### Open file format
The file format used to save decks is extremely easy and straight forward. It's just a text file in JSON format. Your collection is saved in an SQLite database file which can be opened by applications like [sqlitebrowser](http://sqlitebrowser.org/).

Application Files and Folder
----------------------------

Depending on your OS, the configuration files and default application folder have different locations. Here's an overview.

### Mac OS X

#### Secret configuration file
	~/Library/Preferences/be.stijn-v.MTGCards.plist

#### Application Folder
	~/Library/Application Support/stijnvsoftware/MTGCards

### Windows

#### Secret configuration file
Open regedit.exe and go to:
	
	HKEY_CURRENT_USER\Software\stijnvsoftware\MTGCards

#### Application Folder
	C:\Users\**user**\AppData\Local\stijnvsoftware\MTGCards

### Linux

#### Secret configuration file
	~/.config/stijnvsoftware/MTGCards.conf

#### Application Folder
	~/.local/share/stijnvsoftware/MTGCards
	