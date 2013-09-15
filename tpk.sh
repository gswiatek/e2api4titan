#!/bin/bash
PREFIX=titan-plugin
NAME=e2webserv
MAJOR=`cat include/Version.h | grep major | awk -F"= " '{ print $2 }' | awk -F";" '{ print $1 }'`
MINOR=`cat include/Version.h | grep major | awk -F"= " '{ print $2 }' | awk -F";" '{ print $1 }'`
BUILD=`cat include/Version.h | grep build | awk -F"= " '{ print $2 }' | awk -F";" '{ print $1 }'`
VERSION="$MAJOR.$MINOR.$BUILD"

cp $NAME tpk-raw/flash/mnt/bin
cp $NAME tpk-raw/swap/var/swap/bin

tpk() {
	dir=$1
	echo process directory $dir
	mkdir -p tmp
	rm -rf tmp/*
	cp -r tpk-raw/$dir tmp

	section=`cat tmp/$dir/CONTROL/control | grep Section | awk '{ print $2 }'`
	size=`du -c tmp | grep total | cut -f1`
	echo plugin size $size KB
	
	sed -i "s/<VERSION>/$VERSION/" tmp/$dir/CONTROL/control
	sed -i "s/<PLUGIN_SIZE>/$size/" tmp/$dir/CONTROL/control
	sed -i "s/<PLUGIN_SIZE>/$size/" tmp/$dir/CONTROL/preinst

	fileName="${PREFIX}-${section}-${NAME}_${VERSION}_sh4.tpk"
	echo file $fileName
	
	$JAVA_HOME/bin/java -jar java/tpk.jar "tmp/$dir" "tpk/$fileName"
	gzip -f "tpk/$fileName"

}

tpk flash
tpk swap

rm tpk-raw/flash/mnt/bin/$NAME
rm tpk-raw/swap/var/swap/bin/$NAME


