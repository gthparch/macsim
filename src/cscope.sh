#!/bin/sh

if [ -f 'cscope.files' ] 
then
  rm -f 'cscope.files'
fi

find -name '*.cc'   > 'cscope.files'
find -name '*.cpp' >> 'cscope.files'
find -name '*.h'   >> 'cscope.files'
cscope -b
