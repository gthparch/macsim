#!/bin/sh
# Download PIN since XED comes bundled with PIN 

arch=$(uname -m)
TARGET_ARCH=""
if [ $arch = "i686" ]
then
  TARGET_ARCH="ia32"
else
        if [ $arch = "x86_64" ]
        then
                TARGET_ARCH="intel64"
        fi
fi

XED=xed-$TARGET_ARCH
PIN=pin-2.14-71313-gcc.4.4.7-linux
PIN_ARCHIVE=$PIN.tar.gz
PIN_URL=https://software.intel.com/sites/landingpage/pintool/downloads/$PIN_ARCHIVE

UNPACK="tar -xzf"

# Only download it if we don't already have it.
if [ ! -e $PIN_ARCHIVE ]; then
  echo === DOWNLOADING ARCHIVE ===
  wget $PIN_URL
fi

# If the directory already exists, remove it and unpack the archive again.
if [ -e ../$XED ]; then
  rm -r ../$XED
fi

echo === UNPACKING ARCHIVE ===
$UNPACK $PIN_ARCHIVE
mv $PIN/extras/$XED ../
rm -r $PIN
echo === DONE ===
