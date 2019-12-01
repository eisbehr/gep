#! /bin/bash
basedir="$(dirname "$(readlink -f "$0")")"
pushd "$basedir"

if [ "$#" -ne 1 ]; then
	echo "usage: release.sh <release name>"
	exit 1
fi

outputname="$1"
function pack_release {
	echo "doing release packing"
	mkdir -p release
	pushd release
	if [ -e "$outputname" ]; then
		rm -r "$outputname"
	fi
	if [ -e "$outputname.zip" ]; then
		rm -r "$outputname.zip"
	fi
	zipfilepath="$(readlink -f "$outputname.zip")"
	mkdir -p "$outputname"
	pushd "$outputname"
	cp "$basedir/bin/gep.exe" .
	cp "$basedir/bin/SDL2.DLL" .
	cp -r "$basedir/includes" .
	cp "$basedir/LICENSE.txt" .
	cp "$basedir/README.md" .
	mkdir -p demos/flappy
	pushd demos/flappy
	cp "$basedir/bin/gep.exe" .
	cp "$basedir/bin/SDL2.DLL" .
	cp "$basedir/demos/flappy/bin/game.dll" .
	popd
	
	popd
	zip -r "$outputname.zip" "$outputname/"
	if [ -e "$outputname" ]; then
		rm -r "$outputname"
	fi	
	popd
}

echo "This script assumes that all the bin/ paths exist and contain the release build files"
while true; do
    read -p "Continue? (y/n): " -n1 yn
	echo ""
    case $yn in
        [Yy]* ) pack_release; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y or n.";;
    esac
done

popd
