#!/bin/sh
# Download and compile DISTORM 

VERSION=3.4.1
DISTORM=distorm3-$VERSION
DISTORM_ARCHIVE=$DISTORM.tar.gz
DISTORM_URL=https://github.com/gdabah/distorm/releases/download/v$VERSION/$DISTORM_ARCHIVE
UNPACK="tar -xzf"

# Only download it if we don't already have it.
if [ ! -e $DISTORM_ARCHIVE ]; then
  echo === DOWNLOADING ARCHIVE ===
  wget $DISTORM_URL
fi

# If the directory already exists, remove it and unpack the archive again.
if [ -e ../$DISTORM ]; then
  rm -r ../$DISTORM
fi

echo === UNPACKING ARCHIVE ===
$UNPACK $DISTORM_ARCHIVE
cd $DISTORM
patch setup.py < ../patch_distorm_setup.txt
python setup.py -q build_clib
cd ..
mv $DISTORM ../
echo === DONE ===
