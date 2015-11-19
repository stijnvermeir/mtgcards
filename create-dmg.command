#!/bin/bash

cd "$( dirname "${BASH_SOURCE[0]}" )"
cd build/release
~/Qt/5.2.1/clang_64/bin/macdeployqt MTGCards.app -dmg
mv MTGCards.dmg ~/Desktop