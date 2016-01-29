#!/bin/sh
# Download PIN since XED comes bundled with PIN 

XED=xed2-intel64
PIN=pin-2.12-56759-gcc.4.4.7-linux
PIN_ARCHIVE=$PIN.tar.gz
PIN_URL=http://download-software.intel.com/sites/landingpage/pintool/downloads/$PIN_ARCHIVE

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
