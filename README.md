# MTGCards

MTGCards is a tool to manage your collection and decks of Magic the Gathering cards.

Main features:

* Does not require update when new MTG sets are released! Thanks to [mtgjson.com](http://mtgjson.com)
* Automatic tracking of used cards in your collection!
* Easy generation of proxies
* Fetch card prices from [magiccardmarket.eu](http://magiccardmarket.eu)
* Integrated card art download feature
* Fast and snappy
* Advanced search possibilities with support for regular expressions
* Cross platform (Mac OS X, Windows, Linux)
* Native desktop application
* Works offline

And it's free and always will be!

![screenshot](http://mtgcards.stijn-v.be/screenshot1.png)

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