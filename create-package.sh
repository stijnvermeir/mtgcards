#!/bin/bash

cd "$( dirname "${BASH_SOURCE[0]}" )"

mkdir -p build/temproot/usr/local/bin
cd build
cp release/MTGCards temproot/usr/local/bin/mtgcards

fpm -s dir \
	-t deb \
	-n mtgcards \
	-v 0.1.1 \
	-m stijnvermeir@users.noreply.github.com \
	--url https://github.com/stijnvermeir/mtgcards \
	--vendor stijnvsoftware \
	--license "GNU GPLv3" \
	--description "MTGCards is a tool to manage your collection and decks of Magic the Gathering cards." \
	-d libqt5core5a \
	-d libqt5gui5 \
	-d libqt5svg5 \
	-d libqt5widgets5 \
	-d libqt5xml5 \
	--prefix / \
	-C temproot/ \
	usr/local/bin/mtgcards

rm -r temproot

# to upload
# package_cloud push stijnv/public/ubuntu/trusty build/mtgcards_0.1.1_amd64.deb

