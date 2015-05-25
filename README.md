# MTGCards

MTGCards is a tool to manage your collection and decks of Magic the Gathering cards.

Main features:

* Does not require update when new MTG sets are released! Thanks to [mtgjson.com](http://mtgjson.com)
* Automatic tracking of used cards in your collection!
* Easy generation of proxies
* Works fast
* Advanced filter with support for regular expressions
* Multi window = multi display friendly
* Highly customizable: create your own columns
* Cross platform
* Native application (does not require any other runtime environment)
* Open file format to store decks and collection (human readable JSON)

And it's free and always will be!

![screenshot](http://download.stijn-v.be/MTGCards-screenshot.png)

Take a look at the [manual](https://github.com/stijnvermeir/mtgcards/blob/master/manual.md) to set up MTGCards during the first run.

Download binaries
-----------------

### Mac OS X

[MTGCards-0.1.0.dmg](http://download.stijn-v.be/MTGCards-0.1.0.dmg)

Once you download the .dmg file, open it and drag and drop MTGCards.app to your Applications.

The app is not signed, because I don't want to pay 99$ for that. So you'll have to right-click (or CTRL-click) on it and choose Open. Then you'll be able to open it. You only have to do this once.

Tested on Mac OS X 10.9.5. Probably works on Yosemite too.

### Windows 64-bit

* [MTGCards-0.1.0-64bit-installer.exe](http://download.stijn-v.be/MTGCards-0.1.0-64bit-installer.exe)
* [vcredist_x64.exe](http://download.stijn-v.be/vcredist_x64.exe)

If the application doesn't start, you probably need to install vcredist_x64.exe as well.

Tested on Windows 7. Probably works on newer versions as well.

### Linux

No binaries available (yet). I will try to create a package on Launchpad for this. For now, see how to build from source.

Build from source
-----------------

MTGCards has only one dependency and that's Qt.

For Mac and Windows you can download [the online installer from their website](http://www.qt.io/download-open-source/).

### Mac

You'll need to install Xcode as well. I'm still on Mavericks (10.9.5) and downloaded Xcode 5.1.1 from <https://developer.apple.com/downloads>. I used Qt version 5.2.1 to build the binaries you can find here.

To create a self-contained OS X application, you need to run macdeployqt. For example:

	~/Qt/5.2.1/clang_64/bin/macdeployqt MTGCards.app -dmg

### Windows

You'll need to install Visual Studio as well. I used Visual Studio 2013 Community, which you can still find [here](http://go.microsoft.com/?linkid=9863608). I used the latest version of Qt (5.4) to build the binaries you can find here.

To gather all the dependencies in one folder, you need to run windeployqt. For example:

1. Copy MTGCards.exe to some empty folder.
2. Open a Visual Studio Developer Command Prompt and go to that folder.
3. Run this command: `windeployqt MTGCards.exe`

### Linux

On Ubuntu or Linux Mint you can just run `sudo apt-get install qt5-default qt-sdk` to get the required development environment.