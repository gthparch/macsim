#!/bin/sh
# Download and compile DISTORM 

DISTORM=distorm3-3
DISTORM_DIST=$DISTORM-sdist
DISTORM_ARCHIVE=$DISTORM_DIST.zip
DISTORM_URL=https://distorm.googlecode.com/files/$DISTORM_ARCHIVE
UNPACK="unzip -q"

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
