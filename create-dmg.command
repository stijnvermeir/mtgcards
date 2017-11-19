#!/bin/bash

cd "$( dirname "${BASH_SOURCE[0]}" )"
VERSION=$(cat mtgcards/VERSION.txt)
cd build/release
sed -i.bak "s|Created by Qt/QMake|$VERSION|g" MTGCards.app/Contents/Info.plist
rm -f MTGCards.app/Contents/Info.plist.bak
~/Qt/5.7/clang_64/bin/macdeployqt MTGCards.app -dmg
mv MTGCards.dmg ~/Desktop/MTGCards-$VERSION.dmg