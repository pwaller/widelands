#!/bin/bash

set -e

if [ "$#" == "0" ]; then
	echo "Usage: $0 <bzr_repo_directory>"
	exit 1
fi

SOURCE_DIR=$1
REVISION=`bzr revno $SOURCE_DIR`
DESTINATION="WidelandsRelease"
TYPE="Release"
if [[ -f $SOURCE_DIR/WL_RELEASE ]]; then
   WLVERSION="$(cat $SOURCE_DIR/WL_RELEASE)"
else
   WLVERSION="r$REVISION"
fi

echo ""
echo "   Source:      $SOURCE_DIR"
echo "   Version:     $WLVERSION"
echo "   Destination: $DESTINATION"
echo "   Type:        $TYPE"
echo ""

function MakeDMG {
   # Sanity check: Make sure Widelands is there.
   test -f $DESTINATION/Widelands.app/Contents/MacOS/widelands

   find $DESTINATION -name ".?*" -exec rm -v {} \;
   UP=$(dirname $DESTINATION)

   echo "Copying COPYING"
   cp $SOURCE_DIR/COPYING  $DESTINATION/COPYING.txt

   echo "Creating DMG ..."
   hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_64bit_$WLVERSION.dmg"
}

function MakeAppPackage {
   echo "Making $DESTINATION/Widelands.app now."
   rm -Rf $DESTINATION/

   mkdir $DESTINATION/
   mkdir $DESTINATION/Widelands.app/
   mkdir $DESTINATION/Widelands.app/Contents/
   mkdir $DESTINATION/Widelands.app/Contents/Resources/
   mkdir $DESTINATION/Widelands.app/Contents/MacOS/
   cp $SOURCE_DIR/pics/widelands.icns $DESTINATION/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications $DESTINATION/Applications

   cat > $DESTINATION/Widelands.app/Contents/Info.plist << EOF
{
   CFBundleName = widelands;
   CFBundleDisplayName = Widelands;
   CFBundleIdentifier = "org.widelands.wl";
   CFBundleVersion = "$WLVERSION";
   CFBundleInfoDictionaryVersion = "6.0";
   CFBundlePackageType = APPL;
   CFBundleSignature = wdld;
   CFBundleExecutable = widelands;
   CFBundleIconFile = "widelands.icns";
}
EOF

   echo "Copying data files ..."
   rsync -Ca $SOURCE_DIR/ $DESTINATION/Widelands.app/Contents/MacOS/ \
      --exclude "build" \
      --exclude "cmake" \
      --exclude "doc" \
      --exclude "locale" \
      --exclude "manual_test" \
      --exclude "po" \
      --exclude "src" \
      --exclude "test" \
      --exclude "utils" \
      --exclude "*.cmake" \
      --exclude "*.py" \
      --exclude "*.sh" \
      --exclude ".*" \
      --exclude "CMakeLists*"

   echo "Copying locales ..."
   rsync -Ca locale $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying binary ..."
   cp -a src/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands

   echo "Copying dynamic libraries ..."
   dylibbundler -od -b -x $DESTINATION/Widelands.app/Contents/MacOS/widelands  -d $DESTINATION/Widelands.app/Contents/libs
}

function BuildWidelands() {
   cmake $SOURCE_DIR $GENERATOR \
      -DCMAKE_CXX_COMPILER:FILEPATH="$(cd $(dirname $0); pwd)/compiler_wrapper.sh" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="10.7" \
      -DCMAKE_OSX_SYSROOT:PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk" \
      -DCMAKE_INSTALL_PREFIX:PATH="$DESTINATION/Widelands.app/Contents/MacOS" \
      -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" \
      -DCMAKE_BUILD_TYPE:STRING="$TYPE" \
      -DWL_INSTALL_PREFIX:STRING="." \
      -DWL_PATHS_ARE_ABSOLUTE:STRING="false" \
      -DCMAKE_PREFIX_PATH:PATH="/usr/local" \
      \
      -DSDL2_LIBRARY:STRING="-L/usr/local/lib /usr/local/lib/libSDL2main.a /usr/local/lib/libSDL2.a -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit" \
      -DSDL2_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DSDL2IMAGE_LIBRARY:STRING="-Wl,/usr/local/lib/libSDL2_image.a -Wl,/usr/local/lib/libjpeg.a" \
      -DSDL2IMAGE_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DPNG_LIBRARY:FILEPATH="/usr/local/opt/libpng/lib/libpng.a" \
      -DPNG_INCLUDE_DIR:PATH="/usr/local/opt/libpng/include" \
      \
      -DSDL2TTF_LIBRARY:STRING="-Wl,/usr/local/opt/freetype/lib/libfreetype.a -Wl,/usr/local/lib/libbz2.a -Wl,/usr/local/lib/libSDL2_ttf.a" \
      -DSDL2TTF_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DSDL2GFX_LIBRARY:FILEPATH="/usr/local/lib/libSDL2_gfx.a" \
      -DSDL2GFX_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DSDL2MIXER_LIBRARY:STRING="-Wl,/usr/local/lib/libvorbisfile.a -Wl,/usr/local/lib/libogg.a -Wl,/usr/local/lib/libvorbis.a -Wl,/usr/local/lib/libSDL2_mixer.a" \
      -DSDL2MIXER_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DSDL2NET_LIBRARY:FILEPATH="/usr/local/lib/libSDL2_net.a" \
      -DSDL2NET_INCLUDE_DIR:PATH="/usr/local/include/SDL2" \
      \
      -DINTL_LIBRARY:STRING="-Wl,/usr/local/opt/libiconv/lib/libiconv.a -Wl,/usr/local/opt/gettext/lib/libintl.a" \
      -DINTL_INCLUDE_DIR:PATH="/usr/local/opt/gettext/include" \
      \
      -DZLIB_LIBRARY:FILEPATH="/usr/local/opt/zlib/lib/libz.a" \
      -DZLIB_INCLUDE_DIR:PATH="/usr/local/include" \

   make -j2

   echo "Done building."
}

BuildWidelands
MakeAppPackage
MakeDMG
